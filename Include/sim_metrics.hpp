#pragma once

#include <cstdint>
#include <unordered_map>

#include "task.hpp"

using namespace std;

namespace os_simulator {
struct CpuMetrics {
 private:
  uint64_t mTotalBusyTicks{0};
  uint64_t mTotalSimulationTicks{0};
  uint64_t mTotalIdleTicks{0};

  uint64_t mContextSwitchCount{0};
  uint64_t mTotalContextSwitchTicks{0};

  uint64_t mTotalTurnaroundTime{0};
  uint64_t mTotalResponseTime{0};
  uint64_t mTotalWaitTime{0};

  size_t mCompletedProcesses{0};

  // NEW: Interval-based JFI variables
  std::unordered_map<uint16_t, uint64_t> mIntervalAllocations;
  double mTotalJfiSum{0.0};
  uint64_t mJfiSampleCount{0};

 public:
  void recordBusyTicks(uint64_t ticks) {
    mTotalBusyTicks += ticks;
    mTotalSimulationTicks += ticks;
  }

  void recordIdleTicks(uint64_t ticks) {
    mTotalIdleTicks += ticks;
    mTotalSimulationTicks += ticks;
  }

  void recordContextSwitch(uint64_t ticks) {
    mContextSwitchCount++;
    mTotalContextSwitchTicks += ticks;
    mTotalSimulationTicks += ticks;
  }

  void recordTaskOpportunity(uint16_t taskId, uint64_t allocatedTicks) {
    mIntervalAllocations[taskId] += allocatedTicks;
  }

  // NEW: Call this every N ticks (e.g., every 10,000 ticks) from your main OS
  // loop
  void evaluateIntervalFairness() {
    if (mIntervalAllocations.empty()) return;

    double sumXi = 0.0;
    double sumSqXi = 0.0;
    double n = static_cast<double>(mIntervalAllocations.size());

    for (const auto &[taskId, ticks] : mIntervalAllocations) {
      double xi = static_cast<double>(ticks);
      sumXi += xi;
      sumSqXi += (xi * xi);
    }

    if (sumSqXi > 0.0) {
      double intervalJfi = (sumXi * sumXi) / (n * sumSqXi);
      mTotalJfiSum += intervalJfi;
      mJfiSampleCount++;
    }

    // Clear allocations to start the next interval fresh
    mIntervalAllocations.clear();
  }

  void recordTaskCompletion(Task *pTask) {
    mTotalTurnaroundTime += pTask->getTurnaroundTime();
    mTotalResponseTime += pTask->getResponseTime();
    mTotalWaitTime += pTask->getWaitingTime();
    mCompletedProcesses++;
  }

  // NEW: Returns the average JFI across all sampled intervals
  [[nodiscard]] double getJainsFairnessIndex() const {
    if (mJfiSampleCount == 0)
      return 1.0;  // Default to perfect fairness if no samples

    return mTotalJfiSum / static_cast<double>(mJfiSampleCount);
  }

  [[nodiscard]] double getThroughput() const {
    if (mTotalSimulationTicks == 0) return 0.0;
    return static_cast<double>(mCompletedProcesses) /
           static_cast<double>(mTotalSimulationTicks);
  }

  [[nodiscard]] double getCpuUtilization() const {
    if (mTotalSimulationTicks == 0) return 0.0;
    return (static_cast<double>(mTotalBusyTicks) /
            static_cast<double>(mTotalSimulationTicks)) *
           100.0;
  }

  [[nodiscard]] double getAvgWaitingTime() const {
    if (mCompletedProcesses == 0) return 0.0;
    return static_cast<double>(mTotalWaitTime) /
           static_cast<double>(mCompletedProcesses);
  }

  [[nodiscard]] double getAvgTurnaroundTime() const {
    if (mCompletedProcesses == 0) return 0.0;
    return static_cast<double>(mTotalTurnaroundTime) /
           static_cast<double>(mCompletedProcesses);
  }

  [[nodiscard]] double getAvgResponseTime() const {
    if (mCompletedProcesses == 0) return 0.0;
    return static_cast<double>(mTotalResponseTime) /
           static_cast<double>(mCompletedProcesses);
  }

  [[nodiscard]] uint64_t getTotalSimulationTicks() const {
    return mTotalSimulationTicks;
  }

  [[nodiscard]] uint64_t getTotalBusyTicks() const { return mTotalBusyTicks; }

  [[nodiscard]] uint64_t getTotalIdleTicks() const { return mTotalIdleTicks; }

  [[nodiscard]] uint64_t getContextSwitchCounts() const {
    return mContextSwitchCount;
  }

  [[nodiscard]] uint64_t getTotalContextSwitchTicks() const {
    return mTotalContextSwitchTicks;
  }

  [[nodiscard]] size_t getCompletedProcessCount() const {
    return mCompletedProcesses;
  }
};

struct MemoryMetrics {
 private:
  uint32_t mTotalPageFaults{0};
  uint32_t mTotalMemoryAccesses{0};

  unordered_map<uint16_t, uint32_t> mPageFaultsPerTask;

 public:
  void init(size_t taskCount) { mPageFaultsPerTask.reserve(taskCount); }

  void recordAccess(uint16_t taskId, uint64_t accessCount) {
    mTotalMemoryAccesses += accessCount;

    mPageFaultsPerTask.try_emplace(taskId, 0);
  }

  void recordPageFault(uint16_t taskId) {
    mTotalPageFaults++;
    mPageFaultsPerTask[taskId]++;
  }

  [[nodiscard]] uint32_t getTotalPageFaults() const { return mTotalPageFaults; }

  [[nodiscard]] uint32_t getTotalAccesses() const {
    return mTotalMemoryAccesses;
  }

  [[nodiscard]] double getPageFaultRate() const {
    if (mTotalMemoryAccesses == 0) return 0.0;

    return static_cast<double>(mTotalPageFaults) / mTotalMemoryAccesses;
  }

  [[nodiscard]] uint32_t getPageFaultsForTask(uint16_t taskId) const {
    auto it = mPageFaultsPerTask.find(taskId);
    if (it != mPageFaultsPerTask.end()) {
      return it->second;
    }
    return 0;
  }

  [[nodiscard]] const std::unordered_map<uint16_t, uint32_t> &
  getPerTaskFaultMap() const {
    return mPageFaultsPerTask;
  }
};

struct OsSimulationMetrics {
  CpuMetrics cpu;
  MemoryMetrics memory;

  void initialize(size_t taskCount) { memory.init(taskCount); }
};

}  // namespace os_simulator
