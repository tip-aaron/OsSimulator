#include <cstdio>
#include <os_engine.hpp>

namespace os_simulation_engine {

OsSimulationEngine::OsSimulationEngine(
    std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
    std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager)
    : mScheduler(std::move(scheduler)),
      mMemoryManager(std::move(memoryManager)) {}

void OsSimulationEngine::scheduleNextEvent(
    uint64_t delay, os_simulation_discrete_event::SimulationEventType type,
    std::optional<uint16_t> pid) {
  os_simulation_discrete_event::SimulationEvent event;
  event.mTimestamp = mCurrentTime + delay;
  event.mEventType = type;
  event.mPid = pid;

  mEventQueue.push(event);
}

void OsSimulationEngine::loadWorkload(
    const std::vector<os_simulation_parser::ProcessWorkload> &rWorkloads,
    const os_simulation_parser::WorkloadParser &rParser) {
  for (const auto &rWorkload : rWorkloads) {
    mScheduler->addProcess(rWorkload.mProcess);

    uint16_t pid = rWorkload.mProcess.getId();
    mProcessTraces[pid] = rParser.parseTraceFile(rWorkload.mTraceFilePath);
    mTraceAccessIndices[pid] = 0;

    scheduleNextEvent(
        rWorkload.mProcess.getArrivalTime(),
        os_simulation_discrete_event::SimulationEventType::PROCESS_ARRIVAL,
        pid);
  }
}

os_simulation_memory::TraceAccess OsSimulationEngine::getNextTraceForProcess(
    uint16_t processId) {
  auto traceIt = mProcessTraces.find(processId);
  auto indexIt = mTraceAccessIndices.find(processId);

  if (traceIt == mProcessTraces.end() || indexIt == mTraceAccessIndices.end()) {
    throw std::runtime_error("No trace file loaded for Process ID " +
                             std::to_string(processId));
  }

  const auto &traces = traceIt->second;
  size_t currentLineIndex = indexIt->second;

  if (currentLineIndex >= traces.size()) {
    fprintf(
        stderr,
        "Warning: Process %u outran its trace file! Defaulting to read 0x0.\n",
        processId);
    return {0x0, os_simulation_memory::MemoryAccessType::READ};
  }

  os_simulation_memory::TraceAccess currentTraceAccess =
      traces[currentLineIndex];
  mTraceAccessIndices[processId]++;

  return currentTraceAccess;
}

void OsSimulationEngine::runSimulation() {
  while (!mEventQueue.empty() && !mScheduler->isFinished()) {
    os_simulation_discrete_event::SimulationEvent event = mEventQueue.top();

    mEventQueue.pop();

    if (event.mTimestamp > mCurrentTime) {
      if (!mIsCpuBusy) {
        mMetrics.timeline.recordCpuBlock(mCurrentTime, event.mTimestamp,
                                         os_simulation_metrics::CpuState::IDLE);
      }

      mCurrentTime = event.mTimestamp;
    }

    switch (event.mEventType) {
      case os_simulation_discrete_event::SimulationEventType::PROCESS_ARRIVAL:
      case os_simulation_discrete_event::SimulationEventType::
          PAGE_FAULT_RESOLVED: {
        mScheduler->readyProcess(event.mPid.value());

        scheduleNextEvent(
            0, os_simulation_discrete_event::SimulationEventType::DISPATCH);

        break;
      }
      case os_simulation_discrete_event::SimulationEventType::DISPATCH: {
        while (
            !mEventQueue.empty() &&
            mEventQueue.top().mTimestamp == mCurrentTime &&
            mEventQueue.top().mEventType ==
                os_simulation_discrete_event::SimulationEventType::DISPATCH) {
          mEventQueue.pop();
        }

        if (!mEventQueue.empty() &&
            mEventQueue.top().mTimestamp == mCurrentTime) {
          scheduleNextEvent(
              0, os_simulation_discrete_event::SimulationEventType::DISPATCH);
          break;
        }

        if (mIsCpuBusy) {
          break;
        }

        auto *pRunningProcess = mScheduler->getNextProcessToRun();

        if (pRunningProcess == nullptr) {
          mIsCpuBusy = false;

          continue;
        }

        pRunningProcess->dispatch(mCurrentTime);

        mIsCpuBusy = true;
        uint16_t runningProcessId = pRunningProcess->getId();

        if (mLastPid.has_value() && mLastPid.value() != runningProcessId) {
          uint64_t switchCost =
              os_simulation_architecture::CONTEXT_SWITCH_TICK_COST;

          if (switchCost > 0) {
            mMetrics.timeline.recordCpuBlock(
                mCurrentTime, mCurrentTime + switchCost,
                os_simulation_metrics::CpuState::CONTEXT_SWITCH);
            mMetrics.cpu.recordContextSwitch(switchCost);

            mCurrentTime += switchCost;
          }
        }

        mLastPid = runningProcessId;
        uint64_t timeToNextEvent = UINT64_MAX;

        if (!mEventQueue.empty()) {
          if (mEventQueue.top().mTimestamp <= mCurrentTime) {
            timeToNextEvent = 0;
          } else {
            timeToNextEvent = mEventQueue.top().mTimestamp - mCurrentTime;
          }
        }

        if (timeToNextEvent == 0) {
          mIsCpuBusy = false;
          pRunningProcess->preempt();
          mScheduler->preemptProcess(pRunningProcess);
          scheduleNextEvent(
              0, os_simulation_discrete_event::SimulationEventType::DISPATCH);
          break;
        }

        uint64_t preemptionDelay =
            mScheduler->getPreemptionDelay(pRunningProcess);
        uint64_t remainingBurstTime = pRunningProcess->getRemainingTime();
        // The DES "Jump" limit:
        // 1. timeToNextEvent: Someone new arrives or a page fault finishes
        // 2. remainingBurstTime: The current process finishes
        // 3. preemptionDelay: The Scheduler says "Time's up, I need to check
        // others"
        uint64_t executionLimit = std::min(
            std::min(timeToNextEvent, remainingBurstTime), preemptionDelay);
        uint64_t ticksExecuted = 0;
        bool pageFaulted = false;

        uint64_t startExecuteTick = mCurrentTime;

        while (ticksExecuted < executionLimit) {
          os_simulation_memory::TraceAccess trace =
              getNextTraceForProcess(runningProcessId);
          bool isHit = mMemoryManager->accessAddress(
              runningProcessId, trace.mVirtualAddress, trace.mAccessType);

          mMetrics.memory.recordAccess(runningProcessId);

          if (!isHit) {
            mMemoryManager->handlePageFault(
                runningProcessId, trace.mVirtualAddress, trace.mAccessType);
            mMetrics.memory.recordPageFault(runningProcessId);
            mMetrics.timeline.recordMemoryEvent(
                startExecuteTick + ticksExecuted, runningProcessId, true);

            pageFaulted = true;

            mTraceAccessIndices[runningProcessId]--;

            break;
          } else {
            mMetrics.timeline.recordMemoryEvent(
                startExecuteTick + ticksExecuted, runningProcessId, false);
          }

          ticksExecuted++;
        }

        mCurrentTime += ticksExecuted;

        mMetrics.timeline.recordCpuBlock(
            startExecuteTick, startExecuteTick + ticksExecuted,
            os_simulation_metrics::CpuState::EXECUTING_PROCESS,
            runningProcessId);
        mMetrics.cpu.recordBusyTime(ticksExecuted);
        mScheduler->updateProcessExecution(pRunningProcess, ticksExecuted,
                                           mCurrentTime);
        if (pageFaulted) {
          pRunningProcess->block();
          scheduleNextEvent(
              os_simulation_architecture::BACKING_STORE_LATENCY_MS,
              os_simulation_discrete_event::SimulationEventType::
                  PAGE_FAULT_RESOLVED,
              runningProcessId);
        } else if (!pRunningProcess->isFinished()) {
          mScheduler->preemptProcess(pRunningProcess);
        } else if (pRunningProcess->isFinished()) {
          mMetrics.cpu.recordProcessCompletion(
              pRunningProcess->getTurnaroundTime(),
              pRunningProcess->getResponseTime(),
              pRunningProcess->getWaitTime());
        }

        mIsCpuBusy = false;

        scheduleNextEvent(
            0, os_simulation_discrete_event::SimulationEventType::DISPATCH);

        break;
      }
    }
  }

  mMetrics.cpu.setTotalSimulationTicks(mCurrentTime);
}

}  // namespace os_simulation_engine
