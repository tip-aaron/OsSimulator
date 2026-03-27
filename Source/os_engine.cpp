#include "os_engine.hpp"

os_simulation_engine::OsSimulationEngine::OsSimulationEngine(
    std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
    std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager)
    : mScheduler(std::move(scheduler)),
      mMemoryManager(std::move(memoryManager)) {}

void os_simulation_engine::OsSimulationEngine::runSimulation() {
  while (!mScheduler->isFinished()) {
    auto* pRunningProcess = mScheduler->getNextProcessToRun();
  }
}

// DO THIS NEXT
