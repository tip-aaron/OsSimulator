#include <os_engine.hpp>

#include "utils/threading.hpp"

os_simulation_engine::OsSimulationEngine::OsSimulationEngine(
    std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
    std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager)
    : mScheduler(std::move(scheduler)),
      mMemoryManager(std::move(memoryManager)) {}

// TODO: Change to DES
void os_simulation_engine::OsSimulationEngine::runSimulation() {
  auto simStartTime = std::chrono::steady_clock::now();
  size_t totalProcesses = mProcessTraces.size();

  os_simulation_threading::ConsoleSpinnerReporter progressReporter(
      [&]() -> size_t { return mMetrics.cpu.getCompletedProcessCount(); });

  progressReporter.start(totalProcesses);

  std::optional<int> lastPid = std::nullopt;

  while (!mScheduler->isFinished()) {
    auto *pRunningProcess = mScheduler->getNextProcessToRun();
    std::optional<int> currentPid =
        pRunningProcess ? std::optional<int>(pRunningProcess->getId())
                        : std::nullopt;
    uint64_t currentTick = mMetrics.cpu.getTotalSimulationTicks();

    if (currentPid != lastPid && lastPid.has_value() &&
        currentPid.has_value()) {
      uint64_t cost = os_simulation_architecture::CONTEXT_SWITCH_TICK_COST;

      for (uint64_t i = 0; i < cost; ++i) {
        mMetrics.timeline.recordCpuEvent(
            currentTick + i, os_simulation_metrics::CpuState::CONTEXT_SWITCH);
        mMetrics.cpu.recordTick(false);
      }

      mMetrics.cpu.recordContextSwitch(cost);
      currentTick = mMetrics.cpu.getTotalSimulationTicks();
    }

    if (pRunningProcess != nullptr) {
      int pid = currentPid.value();
      os_simulation_memory::TraceAccess nextTraceAccess =
          getNextTraceForProcess(pid);
      bool isHit = mMemoryManager->accessAddress(
          pid, nextTraceAccess.mVirtualAddress, nextTraceAccess.mAccessType);

      mMetrics.memory.recordAccess(pid);
      mMetrics.timeline.recordMemoryEvent(currentTick, pid, !isHit);

      if (!isHit) {
        mMetrics.cpu.recordTick(false);
        mMetrics.memory.recordPageFault(pid);
        mMetrics.timeline.recordCpuEvent(currentTick,
                                         os_simulation_metrics::CpuState::IDLE);

        mMemoryManager->handlePageFault(pRunningProcess->getId(),
                                        nextTraceAccess.mVirtualAddress,
                                        nextTraceAccess.mAccessType);
        pRunningProcess->block(
            os_simulation_architecture::BACKING_STORE_LATENCY_MS);

        mTraceAccessIndices[pid]--;
      } else {
        mMetrics.cpu.recordTick(true);
        mMetrics.timeline.recordCpuEvent(
            currentTick, os_simulation_metrics::CpuState::EXECUTING_PROCESS,
            pid);
        mScheduler->executeProcess(pRunningProcess);

        if (pRunningProcess->isFinished()) {
          mMetrics.cpu.recordProcessCompletion(
              pRunningProcess->getTurnaroundTime(),
              pRunningProcess->getResponseTime(),
              pRunningProcess->getWaitTime());
        }
      }
    } else {
      mMetrics.cpu.recordTick(false);
      mMetrics.timeline.recordCpuEvent(currentTick,
                                       os_simulation_metrics::CpuState::IDLE);
    }

    mScheduler->addTick();
    lastPid = currentPid;
  }

  progressReporter.stop();

  auto simEndTime = std::chrono::steady_clock::now();
  float totalSec = std::chrono::duration_cast<std::chrono::milliseconds>(
                       simEndTime - simStartTime)
                       .count() /
                   1000.0f;

  printf("Engine finished crunching in %.2f seconds.\n", totalSec);
}

void os_simulation_engine::OsSimulationEngine::loadWorkload(
    const std::vector<os_simulation_parser::ProcessWorkload> &rWorkloads,
    const os_simulation_parser::WorkloadParser &rParser) {
  for (const auto &rWorkload : rWorkloads) {
    mScheduler->addProcess(rWorkload.mProcess);

    std::vector<os_simulation_memory::TraceAccess> traceAccessVec =
        rParser.parseTraceFile(rWorkload.mTraceFilePath);
    int pid = rWorkload.mProcess.getId();
    mProcessTraces[pid] = std::move(traceAccessVec);

    mTraceAccessIndices[pid] = 0;
  }
}

os_simulation_memory::TraceAccess
os_simulation_engine::OsSimulationEngine::getNextTraceForProcess(
    int processId) {
  auto traceIt = mProcessTraces.find(processId);
  auto indexIt = mTraceAccessIndices.find(processId);

  if (traceIt == mProcessTraces.end() || indexIt == mTraceAccessIndices.end()) {
    throw std::runtime_error("No trace file loaded for Process ID " +
                             std::to_string(processId));
  }

  const auto &traces = traceIt->second;
  size_t currentLineIndex = indexIt->second;

  if (currentLineIndex >= traces.size()) {
    std::cerr << "Process " << processId << " outran its trace file!\n";

    return {0x0, os_simulation_memory::MemoryAccessType::READ};
  }

  os_simulation_memory::TraceAccess currentTraceAccess =
      traces[currentLineIndex];

  mTraceAccessIndices[processId]++;

  return currentTraceAccess;
}
