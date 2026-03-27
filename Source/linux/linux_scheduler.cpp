#include "linux_scheduler.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

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

uint64_t os_simulation_linux_scheduler::LinuxCfsScheduler::calcDelta(
    uint64_t deltaExec, uint64_t weight, uint64_t inverseWeight) {
  if (weight != os_simulation_linux_scheduler_math::NICE_0_LOAD) [[unlikely]] {
    deltaExec =
        __calcDelta(deltaExec, os_simulation_linux_scheduler_math::NICE_0_LOAD,
                    inverseWeight);
  }

  return deltaExec;
}

os_simulation_linux_scheduler::LinuxCfsScheduler::CfsNode::CfsNode(
    const os_simulation_process::Process& p)
    : process(p) {
  int niceValue = __convPriorityToNice(process.getPriority());
  int arrIndex = std::clamp(niceValue + 20, 0, 39);

  mWeight = os_simulation_linux_scheduler_math::SCHED_PRIO_TO_WEIGHT[arrIndex];
  mInverseWeight =
      os_simulation_linux_scheduler_math::SCHED_PRIO_TO_WMULT[arrIndex];
}

void os_simulation_linux_scheduler::LinuxCfsScheduler::updateProcessStates() {
  for (auto& rNode : mNodes) {
    auto& rProcess = rNode.process;

    if ((rProcess.getState() == os_simulation_process::ProcessState::NEW &&
         rProcess.getArrivalTime() <= mCurrentTime) ||
        rProcess.getState() == os_simulation_process::ProcessState::RUNNING) {
      rProcess.setState(os_simulation_process::ProcessState::READY);
    } else if (rProcess.getState() ==
               os_simulation_process::ProcessState::BLOCKED) {
      rProcess.addTick();
    }
  }
}

os_simulation_linux_scheduler::LinuxCfsScheduler::CfsNode*
os_simulation_linux_scheduler::LinuxCfsScheduler::selectNextNode() {
  CfsNode* pNextNode = nullptr;
  uint64_t minVruntime = std::numeric_limits<uint64_t>::max();

  for (auto& rNode : mNodes) {
    if (rNode.process.getState() !=
            os_simulation_process::ProcessState::READY ||
        rNode.mVruntime >= minVruntime) {
      continue;
    }

    minVruntime = rNode.mVruntime;
    pNextNode = &rNode;
  }

  return pNextNode;
}

os_simulation_process::Process*
os_simulation_linux_scheduler::LinuxCfsScheduler::getNextProcessToRun() {
  CfsNode* nextNode = selectNextNode();

  if (nextNode != nullptr) {
    return &(nextNode->process);
  }

  return nullptr;
}

void os_simulation_linux_scheduler::LinuxCfsScheduler::executeProcess(
    os_simulation_process::Process* process) {
  for (auto& rNode : mNodes) {
    if (rNode.process.getId() == process->getId()) {
      rNode.process.setState(os_simulation_process::ProcessState::RUNNING);

      if (rNode.process.getStartTime() == -1) {
        rNode.process.setStartTime(mCurrentTime - 1);
      }

      rNode.process.addTick();

      uint64_t deltaExecNs = 1'000'000;
      rNode.mVruntime +=
          calcDelta(deltaExecNs, rNode.mWeight, rNode.mInverseWeight);

      if (rNode.process.isFinished()) {
        rNode.process.setCompletionTime(mCurrentTime);
      }

      return;
    }
  }
}

void os_simulation_linux_scheduler::LinuxCfsScheduler::addProcess(
    const os_simulation_process::Process& p) {
  mNodes.emplace_back(p);

  // Initialize vruntime. (Processes start in ProcessState::NEW by default)
  mNodes.back().mVruntime = mNodes.back().mInverseWeight;
}

void os_simulation_linux_scheduler::LinuxCfsScheduler::addTick() {
  mCurrentTime++;
  updateProcessStates();
}

bool os_simulation_linux_scheduler::LinuxCfsScheduler::isFinished() const {
  for (const auto& node : mNodes) {
    if (!node.process.isFinished()) {
      return false;
    }
  }

  return true;
}

const os_simulation_process::Process&
os_simulation_linux_scheduler::LinuxCfsScheduler::getProcess(int id) const {
  for (const auto& node : mNodes) {
    if (node.process.getId() == id) {
      return node.process;
    }
  }

  throw std::runtime_error("Process with ID " + std::to_string(id) +
                           " not found in scheduler");
}
