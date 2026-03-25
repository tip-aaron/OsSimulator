#pragma once
#include <cstdint>

struct CpuMetrics {
private:
    double cpuUtilization{0.0};
    double throughput{0.0};
    double avgWaitingTime{0.0};
    double turnaroundTime{0.0};
    double responseTime{0.0};

public:
    void setCpuUtilization(double util) { cpuUtilization = util; }
    void setThroughput(double tp)       { throughput = tp; }
    void setAvgWaitingTime(double wait) { avgWaitingTime = wait; }
    void setTurnaroundTime(double ta)   { turnaroundTime = ta; }
    void setResponseTime(double resp)   { responseTime = resp; }

    [[nodiscard]] double getCpuUtilization() const { return cpuUtilization; }
    [[nodiscard]] double getThroughput() const     { return throughput; }
    [[nodiscard]] double getAvgWaitingTime() const { return avgWaitingTime; }
    [[nodiscard]] double getTurnaroundTime() const { return turnaroundTime; }
    [[nodiscard]] double getResponseTime() const   { return responseTime; }
};

struct MemoryMetrics {
private:
    uint32_t totalPageFaults{0};
    uint32_t totalMemoryAccesses{0};

public:
    void recordAccess(bool isPageFault) {
        totalMemoryAccesses++;

        if (isPageFault) {
            totalPageFaults++;
        }
    }

    [[nodiscard]] uint32_t getTotalPageFaults() const { return totalPageFaults; }
    [[nodiscard]] uint32_t getTotalAccesses() const   { return totalMemoryAccesses; }
    [[nodiscard]] double getPageFaultRate() const {
        if (totalMemoryAccesses == 0) return 0.0;

        return static_cast<double>(totalPageFaults) / totalMemoryAccesses;
    }
};

struct OsSimulationMetrics {
    CpuMetrics cpu;
    MemoryMetrics memory;
};
