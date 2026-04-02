#include <algorithm>
#include <constants.hpp>
#include <iostream>
#include <linux_scheduler.hpp>
#include <memory>
#include <scheduler.hpp>

#include "rbtree.hpp"
#include "task.hpp"

using namespace std;
using namespace os_simulator;

static uint64_t __calcDelta(uint64_t deltaExec, uint64_t weight,
                            uint64_t inverseWeight) {
  return (deltaExec * (weight * inverseWeight)) >> WMULT_SHIFT;
}

uint64_t LinuxScheduler::calcDelta(uint64_t deltaExec, uint32_t weight,
                                   uint64_t inverseWeight) {
  if (weight != NICE_0_LOAD) [[unlikely]] {
    deltaExec = __calcDelta(deltaExec, NICE_0_LOAD, inverseWeight);
  }

  return deltaExec;
}

[[nodiscard]] shared_ptr<LinuxTaskNode> LinuxScheduler::findNode(
    const uint16_t taskId) const {
  auto it = mNodeMap.find(taskId);

  return (it != mNodeMap.end()) ? it->second : nullptr;
}

void LinuxScheduler::addTask(unique_ptr<Task> pTask) {
  auto taskId = pTask->getId();
  auto node = make_shared<LinuxTaskNode>(std::move(pTask));

  node->vruntime = node->inverseWeight;

  mNodeMap.emplace(taskId, node);
}

void LinuxScheduler::wakeTask(Task *pTask) {
  shared_ptr<LinuxTaskNode> pNode = findNode(pTask->getId());

  if (!pNode) [[unlikely]] {
    throw std::runtime_error("Attempted to wake a task that doesn't exist.");
  }

  if (pNode->inRunQueue) [[unlikely]] {
    mRunQueue.remove(TaskHandle{pNode});
    mTotalRunqueueWeight -= pNode->weight;
    pNode->inRunQueue = false;
  }

  if (pTask->getState() != TaskState::NEW) {
    uint64_t latencyNs = LINUX_TARGET_LATENCY_MS * LINUX_DELTA_EXEC_NS;

    uint64_t sleepBonusVruntime =
        (mMinVruntime > latencyNs) ? (mMinVruntime - latencyNs) : 0;

    pNode->vruntime = max(pNode->vruntime, sleepBonusVruntime);
  }

  // Wrap the pointer in a TaskHandle
  mRunQueue.insert(TaskHandle{pNode});
  mTotalRunqueueWeight += pNode->weight;
  pNode->inRunQueue = true;
}

bool LinuxScheduler::runQueueEmpty() { return mRunQueue.empty(); }

Task *LinuxScheduler::getTask(uint16_t taskId) {
  auto it = mNodeMap.find(taskId);

  return it == mNodeMap.end() ? nullptr : it->second->task.get();
}

[[nodiscard]] Task *LinuxScheduler::getNextTask() {
  if (mRunQueue.empty()) {
    return nullptr;
  }

  // Extract the wrapper, then access the raw pointer inside
  TaskHandle nextHandle = mRunQueue.extractMinimum();
  shared_ptr<LinuxTaskNode> pNextNode = nextHandle.pTask;

  mTotalRunqueueWeight -= pNextNode->weight;
  pNextNode->inRunQueue = false;

  return pNextNode->task.get();
}
[[nodiscard]] uint64_t LinuxScheduler::calculateTimeSlice(Task *pTask) const {
  shared_ptr<LinuxTaskNode> pNode = findNode(pTask->getId());

  uint64_t calcTotalWeight = mTotalRunqueueWeight;

  if (pNode && !pNode->inRunQueue) {
    calcTotalWeight += pNode->weight;
  }

  if (!pNode || calcTotalWeight == 0) [[unlikely]] {
    return LINUX_MIN_GRANULARITY_MS;
  }

  uint64_t nr_running = mRunQueue.size() + 1;
  uint64_t target_latency = LINUX_TARGET_LATENCY_MS;
  uint64_t sched_nr_latency =
      LINUX_TARGET_LATENCY_MS / LINUX_MIN_GRANULARITY_MS;

  if (nr_running > sched_nr_latency) {
    target_latency = nr_running * LINUX_MIN_GRANULARITY_MS;
  }

  uint64_t timeSlice = (target_latency * pNode->weight) / calcTotalWeight;

  return max(timeSlice, LINUX_MIN_GRANULARITY_MS);
}

void LinuxScheduler::onTaskExecuted(Task *pTask, uint64_t elapsedTicks) {
  shared_ptr<LinuxTaskNode> pNode = findNode(pTask->getId());

  if (!pNode) [[unlikely]] {
    return;
  }

  if (pNode->inRunQueue) {
    mRunQueue.remove(TaskHandle{pNode});
    pNode->inRunQueue = false;
    mTotalRunqueueWeight -= pNode->weight;
  }

  uint64_t executionTimeNs = (uint64_t)elapsedTicks * LINUX_DELTA_EXEC_NS;

  uint64_t deltaVruntime =
      calcDelta(executionTimeNs, pNode->weight, pNode->inverseWeight);
  pNode->vruntime += deltaVruntime;
  TaskState state = pTask->getState();
  uint64_t vleft = pNode->vruntime;

  if (!mRunQueue.empty()) {
    // Compare running task against the best waiting task
    vleft = min(vleft, mRunQueue.peekMin().pTask->vruntime);
  }

  // CRITICAL: Strictly monotonic. It can never go backwards!
  mMinVruntime = max(mMinVruntime, vleft);

  switch (state) {
    case TaskState::READY:
    case TaskState::RUNNING: {
      mRunQueue.insert(TaskHandle{pNode});
      pNode->inRunQueue = true;
      mTotalRunqueueWeight += pNode->weight;
      break;
    }
    default: {
    }
  }
}

[[nodiscard]] bool LinuxScheduler::canPreempt(Task *pRunningTask,
                                              Task *pNewTask) const {
  if (!pRunningTask || !pNewTask) {
    return false;
  }

  auto pRunningNode = findNode(pRunningTask->getId());
  auto pNewNode = findNode(pNewTask->getId());

  if (!pRunningNode || !pNewNode) [[unlikely]] {
    return false;
  }

  uint64_t granularityNs = LINUX_MIN_GRANULARITY_MS * LINUX_DELTA_EXEC_NS;

  return (pNewNode->vruntime + granularityNs) < pRunningNode->vruntime;
}

// When a task blocks or finishes:
void LinuxScheduler::removeTask(uint16_t taskId) {
  auto it = mNodeMap.find(taskId);

  if (it != mNodeMap.end()) {
    shared_ptr<LinuxTaskNode> pNode = it->second;

    if (pNode->inRunQueue) {
      mRunQueue.remove(TaskHandle{pNode});

      if (mTotalRunqueueWeight >= pNode->weight) {
        mTotalRunqueueWeight -= pNode->weight;
      }

      pNode->inRunQueue = false;
    }

    mNodeMap.erase(it);
  }
}
