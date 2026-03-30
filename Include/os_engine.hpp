#pragma once

#include <cstdint>
#include <cstdio>
#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "architecture_config.hpp"
#include "discrete_event.hpp"
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

  const os_simulation_metrics::OsSimulationMetrics &getMetricsconst() const {
    return mMetrics;
  }

 private:
  std::shared_ptr<os_simulation_scheduler::IScheduler> mScheduler;
  std::shared_ptr<os_simulation_memory::IMemoryManager> mMemoryManager;
  os_simulation_metrics::OsSimulationMetrics mMetrics;

  std::unordered_map<uint16_t, std::vector<os_simulation_memory::TraceAccess>>
      mProcessTraces;
  std::unordered_map<uint64_t, size_t> mTraceAccessIndices;

  std::priority_queue<
      os_simulation_discrete_event::SimulationEvent,
      std::vector<os_simulation_discrete_event::SimulationEvent>,
      std::greater<os_simulation_discrete_event::SimulationEvent>>
      mEventQueue;
  uint64_t mCurrentTime{0};
  std::optional<uint16_t> mLastPid{std::nullopt};
  bool mIsCpuBusy{false};

  os_simulation_memory::TraceAccess getNextTraceForProcess(uint16_t processId);
  void scheduleNextEvent(uint64_t delay,
                         os_simulation_discrete_event::SimulationEventType type,
                         std::optional<uint16_t> pid = std::nullopt);
};
}  // namespace os_simulation_engine
