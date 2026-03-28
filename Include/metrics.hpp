#pragma once
#include <cstdint>
#include <unordered_map>

namespace os_simulation_metrics {
struct CpuMetrics {
 private:
  double mCpuUtilization{0.0};
  double mThroughput{0.0};
  double mAvgWaitingTime{0.0};
  double mTurnaroundTime{0.0};
  double mResponseTime{0.0};

 public:
  void setCpuUtilization(double util) { mCpuUtilization = util; }
  void setThroughput(double tp) { mThroughput = tp; }
  void setAvgWaitingTime(double wait) { mAvgWaitingTime = wait; }
  void setTurnaroundTime(double ta) { mTurnaroundTime = ta; }
  void setResponseTime(double resp) { mResponseTime = resp; }

  [[nodiscard]] double getCpuUtilization() const { return mCpuUtilization; }
  [[nodiscard]] double getThroughput() const { return mThroughput; }
  [[nodiscard]] double getAvgWaitingTime() const { return mAvgWaitingTime; }
  [[nodiscard]] double getTurnaroundTime() const { return mTurnaroundTime; }
  [[nodiscard]] double getResponseTime() const { return mResponseTime; }
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
};
}  // namespace os_simulation_metrics