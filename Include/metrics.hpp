#pragma once
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace os_simulation_metrics {
enum class CpuState { EXECUTING_PROCESS, IDLE, CONTEXT_SWITCH };

struct CpuTimelineEvent {
  uint64_t mStartTick;
  uint64_t mEndTick;
  CpuState mCpuState;
  std::optional<int> mProcessId;
};

struct MemoryTimelineEvent {
  uint64_t mTick;
  int mProcessId;
  bool mWasPageFault;
};

struct TimeSeriesMetrics {
 private:
  std::vector<CpuTimelineEvent> mCpuTimeline;
  std::vector<MemoryTimelineEvent> mMemoryTimeline;

 public:
  void recordCpuEvent(uint64_t tick, CpuState state,
                      std::optional<int> processId = std::nullopt) {
    bool isNewEventBlock = true;

    if (!mCpuTimeline.empty()) {
      auto &lastEvent = mCpuTimeline.back();

      // We only merge if:
      // 1. The state is exactly the same
      // 2. AND the processId is exactly the same (both nullopt, or both the
      // same ID)
      if (lastEvent.mCpuState == state && lastEvent.mProcessId == processId) {
        isNewEventBlock = false;
      }
    }

    if (isNewEventBlock) {
      mCpuTimeline.push_back({tick, tick, state, processId});
    } else {
      mCpuTimeline.back().mEndTick = tick;
    }
  }

  void recordMemoryEvent(uint64_t tick, int processId, bool isPageFault) {
    mMemoryTimeline.push_back({tick, processId, isPageFault});
  }

  [[nodiscard]] const std::vector<CpuTimelineEvent> &getCpuTimeline() const {
    return mCpuTimeline;
  }

  [[nodiscard]] const std::vector<MemoryTimelineEvent> &getMemoryTimeline()
      const {
    return mMemoryTimeline;
  }
};

struct CpuMetrics {
 private:
  uint64_t mTotalBusyTicks{0};
  uint64_t mTotalSimulationTicks{0};

  uint64_t mContextSwitchCount{0};
  uint64_t mTotalContextSwitchTicks{0};

  uint64_t mTotalTurnaroundTime{0};
  uint64_t mTotalResponseTime{0};
  uint64_t mTotalWaitTime{0};
  size_t mCompletedProcesses{0};

 public:
  void recordContextSwitch(uint64_t duration) {
    mContextSwitchCount++;
    mTotalContextSwitchTicks += duration;
  }

  void recordTick(bool isBusy) {
    mTotalSimulationTicks++;
    if (isBusy) {
      mTotalBusyTicks++;
    }
  }

  void recordProcessCompletion(uint64_t turnaround, uint64_t response,
                               uint64_t wait) {
    mTotalTurnaroundTime += turnaround;
    mTotalResponseTime += response;
    mTotalWaitTime += wait;
    mCompletedProcesses++;
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

  // --- Raw Data Getters ---

  [[nodiscard]] uint64_t getTotalSimulationTicks() const {
    return mTotalSimulationTicks;
  }

  [[nodiscard]] uint64_t getTotalBusyTicks() const { return mTotalBusyTicks; }

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

  std::unordered_map<int, uint32_t> mPageFaultsPerProcess;

 public:
  void recordAccess(int processId) {
    mTotalMemoryAccesses++;

    if (mPageFaultsPerProcess.find(processId) == mPageFaultsPerProcess.end()) {
      mPageFaultsPerProcess[processId] = 0;
    }
  }

  void recordPageFault(int processId) {
    mTotalPageFaults++;
    mPageFaultsPerProcess[processId]++;
  }

  [[nodiscard]] uint32_t getTotalPageFaults() const { return mTotalPageFaults; }

  [[nodiscard]] uint32_t getTotalAccesses() const {
    return mTotalMemoryAccesses;
  }

  [[nodiscard]] double getPageFaultRate() const {
    if (mTotalMemoryAccesses == 0) return 0.0;

    return static_cast<double>(mTotalPageFaults) / mTotalMemoryAccesses;
  }

  [[nodiscard]] uint32_t getPageFaultsForProcess(int processId) const {
    auto it = mPageFaultsPerProcess.find(processId);
    if (it != mPageFaultsPerProcess.end()) {
      return it->second;
    }
    return 0;
  }

  [[nodiscard]] const std::unordered_map<int, uint32_t> &getPerProcessFaultMap()
      const {
    return mPageFaultsPerProcess;
  }
};

struct OsSimulationMetrics {
  CpuMetrics cpu;
  MemoryMetrics memory;
  TimeSeriesMetrics timeline;
};
}  // namespace os_simulation_metrics