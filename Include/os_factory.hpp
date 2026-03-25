#pragma once
#include <memory>
#include "memory_api.hpp"
#include "scheduler_api.hpp"

namespace OsSimulationFactory {
    std::unique_ptr<IScheduler> createScheduler();
    std::unique_ptr<IMemoryManager> createMemoryManager();
}

