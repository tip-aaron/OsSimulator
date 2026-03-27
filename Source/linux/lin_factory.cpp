#include <os_factory.hpp>
#include <vector>

#include "linux_scheduler.hpp"
#include "mglru_memory.hpp"

std::unique_ptr<os_simulation_scheduler::IScheduler>
os_simulation_factory::createScheduler() {
  return std::make_unique<os_simulation_linux_scheduler::LinuxCfsScheduler>();
}

std::unique_ptr<os_simulation_memory::IMemoryManager>
os_simulation_factory::createMemoryManager(
    os_simulation_metrics::MemoryMetrics& metrics) {
  return std::make_unique<os_simulation_memory::MglruMemoryManager>(metrics);
}
