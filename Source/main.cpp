#include <algorithm>
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

  std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler =
      os_simulation_factory::createScheduler();
  std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager =
      os_simulation_factory::createMemoryManager();

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

    std::cout << "Successfully loaded " << workloads.size()
              << " processes...\n";
    std::cout << "Preparing all processes for simulation...\n";
    engine.loadWorkload(workloads, parser);
    std::cout << "All processes have been prepared!...\n";

    std::cout << "Executing OS Simulation...\n";
    engine.runSimulation();
    std::cout << "Simulation completed successfully!\n";

    os_simulation_metrics::OsSimulationMetrics metrics = engine.getMetrics();

    printHeader("SIMULATION RESULTS: CPU METRICS (CFS)");
    std::cout << std::left << std::setw(30) << "Total Simulation Ticks:"
              << metrics.cpu.getTotalSimulationTicks() << "\n";
    std::cout << std::left << std::setw(30) << "CPU Utilization:" << std::fixed
              << std::setprecision(2) << (metrics.cpu.getCpuUtilization())
              << "%\n";
    std::cout << std::left << std::setw(30)
              << "Throughput:" << std::defaultfloat
              << metrics.cpu.getThroughput() << " processes/tick\n";
    std::cout << std::left << std::setw(30)
              << "Avg Waiting Time:" << metrics.cpu.getAvgWaitingTime()
              << " ticks\n";
    std::cout << std::left << std::setw(30)
              << "Avg Turnaround Time:" << metrics.cpu.getAvgTurnaroundTime()
              << " ticks\n";
    std::cout << std::left << std::setw(30)
              << "Avg Response Time:" << metrics.cpu.getAvgResponseTime()
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

    auto faultMap = metrics.memory.getPerProcessFaultMap();

    std::vector<std::pair<int, int>> sortedFaults(faultMap.begin(),
                                                  faultMap.end());

    std::sort(sortedFaults.begin(), sortedFaults.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });

    int count = 0;
    for (const auto &[pid, faults] : sortedFaults) {
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
