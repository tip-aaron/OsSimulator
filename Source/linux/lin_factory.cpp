#include <os_factory.hpp>
#include <vector>

#include "./linux_scheduler.hpp"

std::unique_ptr<os_simulation_scheduler::IScheduler>
os_simulation_factory::createScheduler() {
  return std::make_unique<os_simulation_linux_scheduler::LinuxCfsScheduler>();
}

std::unique_ptr<os_simulation_memory::IMemoryManager>
os_simulation_factory::createMemoryManager() {}
