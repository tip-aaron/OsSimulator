
#include <cstdint>
#include <iostream>
#include <linux_memory.hpp>
#include <task.hpp>
#include <tuple>
#include <utility>

#include "constants.hpp"

using namespace std;
using namespace os_simulator;

void LinuxMemoryManager::initializeTaskMemory(Task *pTask) {
  mActiveTasks.insert(pTask->getId());
  mTaskMemoryStates.emplace(piecewise_construct,
                            forward_as_tuple(pTask->getId()),
                            forward_as_tuple());
}

[[nodiscard]] MemoryAccessStatus LinuxMemoryManager::accessMemory(
    Task *pTask, const MemoryPageAccess &rMemoryPageAccess) {
  auto it_state = mTaskMemoryStates.find(pTask->getId());

  if (it_state == mTaskMemoryStates.end()) {
    cerr << "[FATAL] accessMemory called for unknown/deleted Task ID: "
         << pTask->getId() << endl;
    exit(1);
  }

  auto &rState = it_state->second;
  auto it = rState.pageMap.find(rMemoryPageAccess.vpn);

  if (it != rState.pageMap.end()) {
    if (rMemoryPageAccess.type == MemoryPageAccessType::WRITE) {
      it->second.it->isDirty = true;
    }

    promotePage(rState, rMemoryPageAccess.vpn);

    return MemoryAccessStatus::PAGE_HIT;
  }

  return MemoryAccessStatus::PAGE_FAULT;
}

bool LinuxMemoryManager::handlePageFault(
    Task *pTask, const MemoryPageAccess &rMemoryPageAccess) {
  auto it_state = mTaskMemoryStates.find(pTask->getId());

  if (it_state == mTaskMemoryStates.end()) {
    return false;
  }

  auto &rState = it_state->second;

  if (rState.pageMap.find(rMemoryPageAccess.vpn) != rState.pageMap.end()) {
    return false;
  }

  bool dirtEvictionOccured = false;

  if (mGlobalFramesUsed >= mGlobalFrameLimit) {
    dirtEvictionOccured = evictOldest();
  }

  PageNode newNode{pTask->getId(), rMemoryPageAccess.vpn,
                   (rMemoryPageAccess.type == MemoryPageAccessType::WRITE)};

  uint8_t youngGen = LINUX_MGLRU_MAX_GENERATIONS - 1;

  rState.generations[youngGen].push_front(newNode);
  mGlobalGenerations[youngGen].push_front(
      {pTask->getId(), rMemoryPageAccess.vpn});

  rState.pageMap[rMemoryPageAccess.vpn] = {
      youngGen, rState.generations[youngGen].begin(),
      mGlobalGenerations[youngGen].begin()};

  rState.currentFramesUsed++;
  mGlobalFramesUsed++;

  return dirtEvictionOccured;
}

bool LinuxMemoryManager::needsOsAction() {
  if (mGlobalFramesUsed >= mGlobalFrameLimit) {
    // If Generation 0 is empty, we need to age.
    return mGlobalGenerations[0].empty();
  }
  return false;
}

void LinuxMemoryManager::cleanupTaskMemory(Task *task) {
  mActiveTasks.erase(task->getId());

  auto it = mTaskMemoryStates.find(task->getId());

  if (it != mTaskMemoryStates.end() && it->second.currentFramesUsed == 0) {
    mTaskMemoryStates.erase(it);
  }
}

void LinuxMemoryManager::promotePage(LocalMglruState &rState, uint64_t vpn) {
  auto &rLoc = rState.pageMap[vpn];
  uint8_t oldGen = rLoc.generationIndex;
  uint8_t MAX_GEN = LINUX_MGLRU_MAX_GENERATIONS - 1;

  if (oldGen == MAX_GEN) {
    rState.generations[MAX_GEN].splice(rState.generations[MAX_GEN].begin(),
                                       rState.generations[MAX_GEN], rLoc.it);
    mGlobalGenerations[MAX_GEN].splice(mGlobalGenerations[MAX_GEN].begin(),
                                       mGlobalGenerations[MAX_GEN], rLoc.git);
    return;
  }

  rState.generations[MAX_GEN].splice(rState.generations[MAX_GEN].begin(),
                                     rState.generations[oldGen], rLoc.it);

  mGlobalGenerations[MAX_GEN].splice(mGlobalGenerations[MAX_GEN].begin(),
                                     mGlobalGenerations[oldGen], rLoc.git);

  rLoc.generationIndex = MAX_GEN;
}

void LinuxMemoryManager::ageGenerations(LocalMglruState &rState) {
  for (int i = 1; i < LINUX_MGLRU_MAX_GENERATIONS; ++i) {
    if (rState.generations[i].empty()) {
      continue;
    }

    for (auto &page : rState.generations[i]) {
      auto &rLoc = rState.pageMap.at(page.vpn);

      if (rLoc.generationIndex == i) {
        rLoc.generationIndex = i - 1;

        mGlobalGenerations[i - 1].splice(mGlobalGenerations[i - 1].begin(),
                                         mGlobalGenerations[i], rLoc.git);
      }
    }

    rState.generations[i - 1].splice(rState.generations[i - 1].begin(),
                                     rState.generations[i]);
  }
}

bool LinuxMemoryManager::evictOldest() {
  for (uint8_t i = 0; i < LINUX_MGLRU_MAX_GENERATIONS; ++i) {
    auto &globalList = mGlobalGenerations[i];
    auto it = globalList.end();

    while (it != globalList.begin()) {
      --it;

      uint16_t victimId = it->first;
      uint64_t vpn = it->second;

      bool isDead = mActiveTasks.find(victimId) == mActiveTasks.end();

      auto &victimState = mTaskMemoryStates.at(victimId);
      auto &pageLoc = victimState.pageMap.at(vpn);
      bool wasDirty = pageLoc.it->isDirty;

      victimState.generations[pageLoc.generationIndex].erase(pageLoc.it);
      victimState.pageMap.erase(vpn);
      victimState.currentFramesUsed--;

      if (isDead && victimState.currentFramesUsed == 0) {
        mTaskMemoryStates.erase(victimId);
      }

      globalList.erase(it);
      mGlobalFramesUsed--;

      return wasDirty;
    }
  }

  return false;
}

void LinuxMemoryManager::performOsAction() {
  if (mTaskMemoryStates.empty() ||
      mGlobalFramesUsed !=
          (mGlobalFrameLimit * LINUX_MGLRU_PROACTIVE_AGING_WATERMARK)) {
    return;
  }

  for (auto &[rTaskId, rState] : mTaskMemoryStates) {
    ageGenerations(rState);
  }
}