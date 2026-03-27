#pragma once

#include <memory>
#include <vector>

#include "architecture_config.hpp"
#include "memory_api.hpp"
#include "scheduler_api.hpp"

namespace os_simulation_engine {
struct TraceAccess {
  uint64_t mVirtualAddress;
  os_simulation_memory::MemoryAccessType accessType;
};

class OsSimulationEngine {
 public:
  OsSimulationEngine(
      std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
      std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager);

  void runSimulation();

  TraceAccess getNextTraceForProcess(int processId);

 private:
  std::shared_ptr<os_simulation_scheduler::IScheduler> mScheduler;
  std::shared_ptr<os_simulation_memory::IMemoryManager> mMemoryManager;
};
}  // namespace os_simulation_engine
