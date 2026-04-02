#pragma once

#include <memory>
#include <vector>

#include "events.hpp"
#include "memory.hpp"
#include "scheduler.hpp"
#include "sim_metrics.hpp"
#include "task.hpp"

using namespace std;

namespace os_simulator {

class SimulationEngine {
 public:
  SimulationEngine(shared_ptr<IScheduler> scheduler,
                   shared_ptr<IMemoryManager> memoryManager,
                   vector<unique_ptr<Task>> tasks);

  void run();

  OsSimulationMetrics *getMetrics() { return mMetrics.get(); }

 private:
  unique_ptr<SimulationEventQueue> eventQueue;
  shared_ptr<IScheduler> scheduler;
  shared_ptr<IMemoryManager> memoryManager;

  void handleTaskArrival(Task *task);
  void handleTaskDispatch(Task *task);
  void handleTaskMemoryAccess(Task *task);
  /*
   * @brief Responsible for handling memory
   * page faults. All its role should be:
   *
   * 1. Handle the respective page fault (memory manager).
   * 2. Tell the task it's blocked and cannot access the memory.
   * 3. Tell the cpu (or scheduler) to dispatch the task again after
   * the backing store's latency (simulates the delay from memory to hard
   * drive).
   */
  void handleMemoryPageFault(Task *task);
  void handleMemoryPageHit(uint64_t timeSlice, Task *task);

  uint64_t getElapsedTicks(Task *task) const;

  void executeTask(uint64_t elapsed, Task *taskz);

  /**
   * Task being run by the CPU.
   */
  Task *mRunningTask{nullptr};
  uint64_t mCurrentSimulationTicks;

  unique_ptr<OsSimulationMetrics> mMetrics;
};

}  // namespace os_simulator