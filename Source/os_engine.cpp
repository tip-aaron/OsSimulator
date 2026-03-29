#include <os_engine.hpp>

#include "utils/threading.hpp"

os_simulation_engine::OsSimulationEngine::OsSimulationEngine(
    std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
    std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager)
    : mScheduler(std::move(scheduler)),
      mMemoryManager(std::move(memoryManager)) {}

void os_simulation_engine::OsSimulationEngine::runSimulation() {
  auto simStartTime = std::chrono::steady_clock::now();
  size_t totalProcesses = mProcessTraces.size();

  os_simulation_threading::ConsoleSpinnerReporter progressReporter(
      [&]() -> size_t {
        size_t completeCount = 0;

        for (const auto &[pid, traces] : mProcessTraces) {
          const auto &proc = mScheduler->getProcess(pid);

          if (proc != nullptr && proc->getCompletionTime() > 0) {
            completeCount++;
          }
        }

        return completeCount;
      });

  progressReporter.start(totalProcesses);

  while (!mScheduler->isFinished()) {
    auto *pRunningProcess = mScheduler->getNextProcessToRun();

    if (pRunningProcess != nullptr) {
      int pid = pRunningProcess->getId();
      os_simulation_memory::TraceAccess nextTraceAccess =
          getNextTraceForProcess(pid);
      bool isHit = mMemoryManager->accessAddress(
          pid, nextTraceAccess.mVirtualAddress, nextTraceAccess.mAccessType);

      if (!isHit) {
        mMemoryManager->handlePageFault(pRunningProcess->getId(),
                                        nextTraceAccess.mVirtualAddress,
                                        nextTraceAccess.mAccessType);
        pRunningProcess->block(
            os_simulation_architecture::BACKING_STORE_LATENCY_MS);

        mTraceAccessIndices[pid]--;
      } else {
        mScheduler->executeProcess(pRunningProcess);
      }
    }

    mScheduler->addTick();
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
