#include "mglru_memory.hpp"

#include <algorithm>
#include <architecture_config.hpp>
#include <iterator>

os_simulation_memory::MglruMemoryManager::MglruMemoryManager()
    : mFreeFrames(os_simulation_architecture::PHYSICAL_FRAME_COUNT) {
  mGenerations.resize(NUM_GENERATIONS);
}

uint64_t os_simulation_memory::MglruMemoryManager::getVpn(
    uint64_t virtualAddress) const {
  return virtualAddress / os_simulation_architecture::PAGE_SIZE_BYTES;
}

bool os_simulation_memory::MglruMemoryManager::accessAddress(
    int processId, uint64_t virtualAddress, MemoryAccessType accessType) {
  uint64_t vpn = getVpn(virtualAddress);
  auto it = mPageTable.find(GlobalPageId{processId, vpn});

  if (it != mPageTable.end()) {
    it->second.mReferenced = true;

    if (accessType == MemoryAccessType::WRITE) {
      it->second.mDirty = true;
    }

    return true;
  }
  return false;
}

void os_simulation_memory::MglruMemoryManager::handlePageFault(
    int processId, uint64_t virtualAddress, MemoryAccessType accessType) {
  uint64_t vpn = getVpn(virtualAddress);
  GlobalPageId globalId{processId, vpn};

  if (mFreeFrames == 0) {
    evictPage();
  } else {
    mFreeFrames--;
  }

  mGenerations[0].push_front(globalId);

  mPageTable.emplace(
      globalId,
      PageEntry{/* .mGeneration   = */ 0,
                /* .mReferenced   = */ false,
                /* .mDirty        = */ accessType == MemoryAccessType::WRITE,
                /* .mListIterator = */ mGenerations[0].begin()});
}

void os_simulation_memory::MglruMemoryManager::ageGenerations() {
  std::vector<std::list<GlobalPageId>> nextGenerations(NUM_GENERATIONS);

  for (int g = 0; g < NUM_GENERATIONS; ++g) {
    auto it = mGenerations[g].begin();

    while (it != mGenerations[g].end()) {
      const GlobalPageId globalId = *it;
      auto &entry = mPageTable.at(globalId);
      auto next_it = std::next(it);

      if (entry.mReferenced) {
        entry.mGeneration = 0;
        entry.mReferenced = false;

        nextGenerations[0].splice(nextGenerations[0].begin(), mGenerations[g],
                                  it);
      } else {
        const int nextGen = std::min(g + 1, NUM_GENERATIONS - 1);
        entry.mGeneration = nextGen;

        nextGenerations[nextGen].splice(nextGenerations[nextGen].end(),
                                        mGenerations[g], it);
      }

      it = next_it;
    }
  }

  mGenerations = std::move(nextGenerations);
}

void os_simulation_memory::MglruMemoryManager::evictPage() {
  for (int g = NUM_GENERATIONS - 1; g >= 0; --g) {
    if (mGenerations[g].empty()) {
      continue;
    }

    const GlobalPageId idToEvict = mGenerations[g].back();

    mPageTable.erase(idToEvict);
    mGenerations[g].pop_back();

    mFreeFrames++;

    return;
  }
}
