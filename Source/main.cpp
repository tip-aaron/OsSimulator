#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>

#include "metrics.hpp"
#include "os_engine.hpp"
#include "os_factory.hpp"
#include "workload_parser.hpp"

void printHeader(const std::string &title) {
  std::cout << "\n======================================================\n";
  std::cout << "  " << title << "\n";
  std::cout << "======================================================\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <PROJECT_ROOT_PATH>\n";
    std::cerr << "Example: " << argv[0] << " ../workloads/\n";

    return 1;
  }

  std::string projectRoot = argv[1];

  std::cout << "Starting simulation with project root: " << projectRoot << "\n";

  os_simulation_metrics::OsSimulationMetrics metrics;

  std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler =
      os_simulation_factory::createScheduler();
  std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager =
      os_simulation_factory::createMemoryManager(metrics.memory);

  os_simulation_engine::OsSimulationEngine engine(scheduler, memoryManager);

  try {
    os_simulation_parser::WorkloadParser parser(projectRoot);

    auto currentWorkload =
        os_simulation_parser::WorkloadType::MIXED_INTERACTIVE_BACKGROUND;

    printHeader("SIMULATION INITIALIZATION");
    std::cout << "Parsing workload scenario...\n";

    auto workloads = parser.parse(currentWorkload);

    if (workloads.empty()) {
      std::cerr
          << "Warning: No workloads loaded. Check your CSV and trace files.\n";
      return 1;
    }

    std::cout << "Successfully loaded " << workloads.size() << " processes.\n";
    engine.loadWorkload(workloads, parser);

    std::cout << "Executing OS Simulation...\n";
    engine.runSimulation();
    std::cout << "Simulation completed successfully!\n";

    double totalTurnaround = 0.0;
    double totalWait = 0.0;
    double totalResponse = 0.0;
    uint64_t maxCompletionTime = 0;
    uint64_t minArrivalTime = std::numeric_limits<uint64_t>::max();
    uint64_t totalBurst = 0;

    for (const auto &wl : workloads) {
      // Assuming ProcessWorkload struct has a member named `mProcess` or
      // `process` Adjust if your struct member is named differently (e.g.,
      // wl.process.getId())
      int pid = wl.mProcess.getId();

      // Retrieve the final process state from the scheduler
      const auto &finishedProc = scheduler->getProcess(pid);

      uint64_t arrival = finishedProc->getArrivalTime();
      uint64_t completion = finishedProc->getCompletionTime();
      uint64_t burst = finishedProc->getBurstTime();
      uint64_t start = finishedProc->getStartTime();

      uint64_t turnaround = completion - arrival;
      uint64_t wait = turnaround - burst;
      uint64_t response = start - arrival;

      totalTurnaround += turnaround;
      totalWait += wait;
      totalResponse += response;
      totalBurst += burst;

      maxCompletionTime = std::max(maxCompletionTime, completion);
      minArrivalTime = std::min(minArrivalTime, arrival);
    }

    uint64_t totalSimulationTime = maxCompletionTime - minArrivalTime;
    size_t numProcesses = workloads.size();

    if (numProcesses > 0) {
      metrics.cpu.setAvgWaitingTime(totalWait /
                                    static_cast<double>(numProcesses));
      metrics.cpu.setTurnaroundTime(totalTurnaround /
                                    static_cast<double>(numProcesses));
      metrics.cpu.setResponseTime(totalResponse /
                                  static_cast<double>(numProcesses));
    }

    if (totalSimulationTime > 0) {
      metrics.cpu.setThroughput(static_cast<double>(numProcesses) /
                                totalSimulationTime);
      metrics.cpu.setCpuUtilization(static_cast<double>(totalBurst) /
                                    totalSimulationTime);
    }

    printHeader("SIMULATION RESULTS: CPU METRICS (CFS)");
    std::cout << std::left << std::setw(30)
              << "Total Simulation Ticks:" << totalSimulationTime << "\n";
    std::cout << std::left << std::setw(30) << "CPU Utilization:" << std::fixed
              << std::setprecision(2)
              << (metrics.cpu.getCpuUtilization() * 100.0) << "%\n";
    std::cout << std::left << std::setw(30)
              << "Throughput:" << std::defaultfloat
              << metrics.cpu.getThroughput() << " processes/tick\n";
    std::cout << std::left << std::setw(30)
              << "Avg Waiting Time:" << metrics.cpu.getAvgWaitingTime()
              << " ticks\n";
    std::cout << std::left << std::setw(30)
              << "Avg Turnaround Time:" << metrics.cpu.getTurnaroundTime()
              << " ticks\n";
    std::cout << std::left << std::setw(30)
              << "Avg Response Time:" << metrics.cpu.getResponseTime()
              << " ticks\n";

    printHeader("SIMULATION RESULTS: MEMORY METRICS (MGLRU)");
    std::cout << std::left << std::setw(30)
              << "Total Memory Accesses:" << metrics.memory.getTotalAccesses()
              << "\n";
    std::cout << std::left << std::setw(30)
              << "Total Page Faults:" << metrics.memory.getTotalPageFaults()
              << "\n";
    std::cout << std::left << std::setw(30) << "Page Fault Rate:" << std::fixed
              << std::setprecision(4)
              << (metrics.memory.getPageFaultRate() * 100.0) << "%\n";

    // Print Top 5 Processes by Page Faults to give insight into thrashing
    std::cout << "\n--- Top 5 Processes by Page Faults ---\n";
    int count = 0;
    for (const auto &[pid, faults] : metrics.memory.getPerProcessFaultMap()) {
      std::cout << "PID " << std::left << std::setw(5) << pid << ": " << faults
                << " faults\n";
      if (++count >= 5) break;
    }
    std::cout << "======================================================\n";

  } catch (const std::exception &e) {
    std::cerr << "\n[CRITICAL ERROR] " << e.what() << "\n";
    return 1;
  }

  return 0;
}
