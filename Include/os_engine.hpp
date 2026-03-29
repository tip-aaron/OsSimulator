#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "architecture_config.hpp"
#include "memory_api.hpp"
#include "metrics.hpp"
#include "process.hpp"
#include "scheduler_api.hpp"
#include "workload_parser.hpp"

namespace os_simulation_engine {
class OsSimulationEngine {
 public:
  OsSimulationEngine(
      std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
      std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager);

  void runSimulation();

  void loadWorkload(
      const std::vector<os_simulation_parser::ProcessWorkload> &rWorkloads,
      const os_simulation_parser::WorkloadParser &rParser);

  const os_simulation_metrics::OsSimulationMetrics &getMetrics() const {
    return mMetrics;
  }

 private:
  std::shared_ptr<os_simulation_scheduler::IScheduler> mScheduler;
  std::shared_ptr<os_simulation_memory::IMemoryManager> mMemoryManager;

  os_simulation_metrics::OsSimulationMetrics mMetrics;
  uint64_t mTotalBusyTicks{0};
  uint64_t mTotalSimulationTicks{0};

  std::unordered_map<int, std::vector<os_simulation_memory::TraceAccess>>
      mProcessTraces;
  // Maps a Process ID to its current "instruction pointer" (which trace line
  // it's on)
  std::unordered_map<int, size_t> mTraceAccessIndices;

  os_simulation_memory::TraceAccess getNextTraceForProcess(int processId);

  void finalizeCpuMetrics();
};
}  // namespace os_simulation_engine
