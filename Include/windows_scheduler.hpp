#pragma once

#include <cstdint>
#include <string>

#include "scheduler.hpp"

namespace os_simulator {
/**
 * @brief Wrapper node for Windows Priority Scheduler.
 * Holds the simulation task alongside Windows-specific metadata for priority
 * queues.
 */
struct WindowsTaskNode {
  Task *simTask;

  uint8_t basePriority;
  uint8_t currentPriority;

  uint64_t quantumRemaining;

  explicit WindowsTaskNode(Task *t, uint8_t basePrio = 8)
      : simTask(t),
        basePriority(basePrio),
        currentPriority(basePrio),
        quantumRemaining(0) {}
};

class WindowsScheduler : public IScheduler {
 public:
  ~WindowsScheduler() override = default;

  [[nodiscard]] string getAlgorithmName() const override {
    return "Windows Priority Scheduler";
  }

  void addTask(Task *task) override;

  void wakeTask(Task *task) override;

  [[nodiscard]] Task *getNextTask() override;

  [[nodiscard]] uint64_t calculateTimeSlice(Task *task) const override;

  void onTaskExecuted(Task *task, uint64_t elapsedTicks) override;

 private:
  [[nodiscard]] WindowsTaskNode *findNode(const Task *task) const;
};
}  // namespace os_simulator