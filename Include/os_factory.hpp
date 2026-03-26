#pragma once
#include <memory>

#include "memory_api.hpp"
#include "scheduler_api.hpp"

namespace os_simulation_factory {
std::unique_ptr<os_simulation_scheduler::IScheduler> createScheduler();
std::unique_ptr<os_simulation_memory::IMemoryManager> createMemoryManager();
}  // namespace os_simulation_factory
