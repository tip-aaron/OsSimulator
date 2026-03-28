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

// DO THIS NEXT