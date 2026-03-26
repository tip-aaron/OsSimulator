#include <algorithm>
#include <limits>

#include "linux_scheduler.hpp"

namespace {
static uint64_t __calcDelta(uint64_t deltaExec, uint64_t weight,
                            uint64_t inverseWeight) {
  uint64_t fairnessCt = weight * inverseWeight;

  return (deltaExec * fairnessCt) >>
         os_simulation_linux_scheduler_math::WMULT_SHIFT;
}

/**
 * @brief Converts a 1-10 priority scale to Linux's -20 to 19 nice value scale.
 * * This function uses linear interpolation to map a custom priority range
 * [1, 10] to the standard Linux Completely Fair Scheduler (CFS) nice
 * value range [-20, 19].
 * * The mapping assumes:
 * - Priority 1 (highest) maps to Nice -20 (maximum CPU share)
 * - Priority 10 (lowest) maps to Nice 19 (minimum CPU share)
 * * The mathematical formula used is: N = -20 + ((P - 1) * 39) / 9
 * * @param priority The raw priority value, expected to be in the range [1,
 * 10]. Values outside this range are safely clamped.
 * @return int The corresponding Linux nice value in the range [-20, 19].
 */
static int __convPriorityToNice(int priority) {
  return -20 + (std::clamp(priority, 1, 10) - 1) * 39 / 9;
}

}  // namespace

uint64_t os_simulation_linux_scheduler::LinuxCfsScheduler::calcDelta(
    uint64_t deltaExec, uint64_t weight, uint64_t inverseWeight) {
  if (weight != os_simulation_linux_scheduler_math::NICE_0_LOAD) [[unlikely]] {
    deltaExec = __calcDelta(deltaExec, weight, inverseWeight);
  }

  return deltaExec;
}

os_simulation_linux_scheduler::CfsNode::CfsNode(
    const os_simulation_process::Process& p)
    : process(p) {
  int niceValue = __convPriorityToNice(process.getPriority());
  int arrIndex = std::clamp(niceValue + 20, 0, 39);

  weight = os_simulation_linux_scheduler_math::SCHED_PRIO_TO_WEIGHT[arrIndex];
  inverseWeight =
      os_simulation_linux_scheduler_math::SCHED_PRIO_TO_WMULT[arrIndex];
}

void os_simulation_linux_scheduler::LinuxCfsScheduler::addProcess(
    const os_simulation_process::Process& p) {
  mNodes.emplace_back(p);
}

void os_simulation_linux_scheduler::LinuxCfsScheduler::addTick() {
  mCurrentTime++;

  os_simulation_linux_scheduler::CfsNode* nextNode = nullptr;
  uint64_t minVruntime = std::numeric_limits<uint64_t>::max();

  for (auto& node : mNodes) {
    if (node.process.isFinished() ||
        node.process.getArrivalTime() >= mCurrentTime) {
      continue;
    }

    if (node.vruntime < minVruntime) {
      minVruntime = node.vruntime;
      nextNode = &node;
    }
  }

  if (nextNode == nullptr) {
    return;
  }

  nextNode->process.setStartTime(mCurrentTime - 1);
  nextNode->process.addTick();

  uint64_t deltaExec = 1;
  nextNode->vruntime +=
      calcDelta(deltaExec, nextNode->weight, nextNode->inverseWeight);

  if (nextNode->process.isFinished()) {
    nextNode->process.setCompletionTime(mCurrentTime);
  }
}

bool os_simulation_linux_scheduler::LinuxCfsScheduler::isFinished() const {
  for (const auto& node : mNodes) {
    if (!node.process.isFinished()) {
      return false;
    }
  }

  return true;
}