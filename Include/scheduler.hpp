#pragma once

#include <string>

#include "task.hpp"

using namespace std;

namespace os_simulator {
/**
 * @brief The generic interface for all CPU Scheduling algorithms.
 * The Engine interacts exclusively with this interface, remaining blind
 * to whether the underlying logic is Windows or Linux.
 */
class IScheduler {
 public:
  virtual ~IScheduler() = default;

  /**
   * @brief Provides a human-readable name of the algorithm for metrics/UI.
   * Example: "Linux Completely Fair Scheduler (CFS)" or "Windows MLFQ".
   */
  [[nodiscard]] virtual std::string getAlgorithmName() const = 0;

  /**
   * Used purely for adding a task to the scheduler
   */
  virtual void addTask(unique_ptr<Task> task) = 0;

  /**
   * @brief Called by the Engine when a BLOCKED task finishes its Page Fault
   * delay. Separated from addTask because some OS algorithms treat waking tasks
   * differently than brand new tasks (e.g., priority boosts in Windows).
   * @param task Pointer to the task that is now READY.
   */
  virtual void wakeTask(Task *task) = 0;

  virtual bool empty() = 0;

  /**
   * @brief Asks the scheduler who should get the CPU next.
   * @return Pointer to the selected task, or nullptr if the Ready Queue is
   * empty (Idle system).
   */
  [[nodiscard]] virtual Task *getNextTask() = 0;

  /**
   * @brief Asks the scheduler how long the given task is allowed to run.
   * In a DES, the Engine uses this to schedule the PREEMPTION event.
   * - Windows might return a fixed quantum (e.g., 20 or 30 ticks).
   * - Linux CFS calculates this dynamically based on the target latency and
   * number of tasks.
   * @param task The task about to be dispatched.
   * @return The maximum number of ticks the task can run before it must be
   * evaluated.
   */
  [[nodiscard]] virtual uint64_t calculateTimeSlice(Task *task) const = 0;

  /**
   * @brief Called by the Engine after a task finishes a burst (via preemption,
   * block, or completion). This is where the OS algorithms update their
   * internal metadata.
   * - Linux CFS will increase the task's vruntime by elapsedTicks.
   * - Windows might decrease the task's priority if it used its entire time
   * slice.
   * @param task The task that just ran.
   * @param elapsedTicks The actual amount of CPU time the task consumed.
   */
  virtual void onTaskExecuted(Task *task, uint64_t elapsedTicks) = 0;

  virtual void removeTask(uint16_t taskId) = 0;

  [[nodiscard]] virtual bool canPreempt(Task *pRunningTask,
                                        Task *pNewTask) const = 0;

  [[nodiscard]] virtual Task *getTask(uint16_t taskId) = 0;
};
}  // namespace os_simulator
