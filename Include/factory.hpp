#pragma once
#include <memory>

#include "memory.hpp"
#include "scheduler.hpp"

using namespace std;

namespace os_simulator {
shared_ptr<IScheduler> createScheduler();
shared_ptr<IMemoryManager> createMemoryManager();
}  // namespace os_simulator
