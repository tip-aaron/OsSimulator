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
  std::optional<uint16_t> mProcessId;
};

struct MemoryTimelineEvent {
  uint64_t mTick;
  uint16_t mProcessId;
  bool mWasPageFault;
};

struct TimeSeriesMetrics {
 private:
  std::vector<CpuTimelineEvent> mCpuTimeline;
  std::vector<MemoryTimelineEvent> mMemoryTimeline;

 public:
  void initialize(size_t expectedEvents) {
    mCpuTimeline.reserve(expectedEvents);
    mMemoryTimeline.reserve(expectedEvents);
  }

  void recordCpuBlock(uint64_t startTick, uint64_t endTick, CpuState state,
                      std::optional<uint16_t> processId = std::nullopt) {
    if (startTick == endTick) return;

    if (!mCpuTimeline.empty()) {
      auto &rLastEvent = mCpuTimeline.back();
      if (rLastEvent.mCpuState == state && rLastEvent.mProcessId == processId &&
          rLastEvent.mEndTick == startTick) {
        rLastEvent.mEndTick = endTick;
        return;
      }
    }

    mCpuTimeline.push_back({startTick, endTick, state, processId});
  }

  void recordMemoryEvent(uint64_t tick, uint16_t processId, bool isPageFault) {
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

  void recordBusyTime(uint64_t busyDuration) {
    mTotalBusyTicks += busyDuration;
  }

  void setTotalSimulationTicks(uint64_t totalTicks) {
    mTotalSimulationTicks = totalTicks;
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

  std::unordered_map<uint16_t, uint32_t> mPageFaultsPerProcess;

 public:
  void initialize(size_t processCount) {
    mPageFaultsPerProcess.reserve(processCount);
  }

  void recordAccess(uint16_t processId) {
    mTotalMemoryAccesses++;

    mPageFaultsPerProcess.try_emplace(processId, 0);
  }

  void recordPageFault(uint16_t processId) {
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

  [[nodiscard]] uint32_t getPageFaultsForProcess(uint16_t processId) const {
    auto it = mPageFaultsPerProcess.find(processId);
    if (it != mPageFaultsPerProcess.end()) {
      return it->second;
    }
    return 0;
  }

  [[nodiscard]] const std::unordered_map<uint16_t, uint32_t> &
  getPerProcessFaultMap() const {
    return mPageFaultsPerProcess;
  }
};

struct OsSimulationMetrics {
  CpuMetrics cpu;
  MemoryMetrics memory;
  TimeSeriesMetrics timeline;

  void initialize(size_t processCount, size_t expectedTimelineEvents = 10000) {
    memory.initialize(processCount);
    timeline.initialize(expectedTimelineEvents);
  }
};

}  // namespace os_simulation_metrics