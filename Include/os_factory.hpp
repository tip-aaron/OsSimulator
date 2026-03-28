#pragma once
#include <memory>

#include "memory_api.hpp"
#include "metrics.hpp"
#include "scheduler_api.hpp"

namespace os_simulation_factory {
std::unique_ptr<os_simulation_scheduler::IScheduler> createScheduler();
std::unique_ptr<os_simulation_memory::IMemoryManager> createMemoryManager(
    os_simulation_metrics::MemoryMetrics &metrics);
}  // namespace os_simulation_factory
