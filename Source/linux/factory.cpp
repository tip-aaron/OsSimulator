#include <factory.hpp>
#include <linux_memory.hpp>
#include <linux_scheduler.hpp>
#include <memory>

#include "memory.hpp"
#include "scheduler.hpp"

using namespace std;

namespace os_simulator {
shared_ptr<IScheduler> createScheduler() {
  return make_shared<LinuxScheduler>();
}

shared_ptr<IMemoryManager> createMemoryManager() {
  return make_shared<LinuxMemoryManager>();
}
}  // namespace os_simulator
