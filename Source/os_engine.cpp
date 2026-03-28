#include "os_engine.hpp"

os_simulation_engine::OsSimulationEngine::OsSimulationEngine(
    std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
    std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager)
    : mScheduler(std::move(scheduler)),
      mMemoryManager(std::move(memoryManager)) {}

void os_simulation_engine::OsSimulationEngine::runSimulation() {
  while (!mScheduler->isFinished()) {
    auto *pRunningProcess = mScheduler->getNextProcessToRun();

    if (pRunningProcess != nullptr) {
      os_simulation_process::TraceAccess nextTraceAccess =
          getNextTraceForProcess(pRunningProcess->getId());
      bool isHit = mMemoryManager->accessAddress(
          pRunningProcess->getId(), nextTraceAccess.mVirtualAddress,
          nextTraceAccess.mAccessType);

      if (!isHit) {
        mMemoryManager->handlePageFault(pRunningProcess->getId(),
                                        nextTraceAccess.mVirtualAddress,
                                        nextTraceAccess.mAccessType);
        pRunningProcess->block(
            os_simulation_architecture::BACKING_STORE_LATENCY_MS);
      } else {
        mScheduler->executeProcess(pRunningProcess);
      }
    }

    mScheduler->addTick();
  }
}

void os_simulation_engine::OsSimulationEngine::loadWorkload(
    const std::vector<os_simulation_parser::ProcessWorkload> &rWorkloads,
    const os_simulation_parser::WorkloadParser &rParser) {
  for (const auto &rWorkload : rWorkloads) {
    mScheduler->addProcess(rWorkload.mProcess);

    std::vector<os_simulation_process::TraceAccess> traceAccessVec =
        rParser.parseTraceFile(rWorkload.mTraceFilePath);
    int pid = rWorkload.mProcess.getId();
    mProcessTraces[pid] = std::move(traceAccessVec);

    mTraceAccessIndices[pid] = 0;
  }
}

os_simulation_process::TraceAccess
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

  os_simulation_process::TraceAccess currentTraceAccess =
      traces[currentLineIndex];

  mTraceAccessIndices[processId]++;

  return currentTraceAccess;
}

// DO THIS NEXT