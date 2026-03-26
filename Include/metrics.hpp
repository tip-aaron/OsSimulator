#pragma once
#include <cstdint>

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

 public:
  void recordAccess(bool isPageFault) {
    mTotalMemoryAccesses++;

    if (isPageFault) {
      mTotalPageFaults++;
    }
  }

  [[nodiscard]] uint32_t getTotalPageFaults() const { return mTotalPageFaults; }
  [[nodiscard]] uint32_t getTotalAccesses() const {
    return mTotalMemoryAccesses;
  }
  [[nodiscard]] double getPageFaultRate() const {
    if (mTotalMemoryAccesses == 0) return 0.0;

    return static_cast<double>(mTotalPageFaults) / mTotalMemoryAccesses;
  }
};

struct OsSimulationMetrics {
  CpuMetrics cpu;
  MemoryMetrics memory;
};
}  // namespace os_simulation_metrics