#include <algorithm>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

#include "metrics.hpp"
#include "os_engine.hpp"
#include "os_factory.hpp"
#include "workload_parser.hpp"

void printHeader(const std::string &title) {
  printf("\n======================================================\n");
  printf("  %s\n", title);
  printf("======================================================\n");
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <PROJECT_ROOT_PATH>\n", argv[0]);
    fprintf(stderr, "Example: %s ../workloads/\n", argv[0]);

    return 1;
  }

  std::string projectRoot = argv[1];

  printf("Starting simulation with project root: %s\n", projectRoot.c_str());

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
    printf("Parsing workload scenario...\n");

    auto workloads = parser.parse(currentWorkload);

    if (workloads.empty()) {
      std::cerr
          << "Warning: No workloads loaded. Check your CSV and trace files.\n";
      return 1;
    }

    printf("Successfully loaded %zu processes...\n", workloads.size());
    printf("Preparing all processes for simulation...\n");
    engine.loadWorkload(workloads, parser);
    printf("All processes have been prepared!...\n");

    printf("Executing OS Simulation...\n");
    engine.runSimulation();
    printf("Simulation completed successfully!\n");

    os_simulation_metrics::OsSimulationMetrics metrics = engine.getMetrics();

    printHeader("SIMULATION RESULTS: CPU METRICS (CFS)");
    printf("%-30s %llu\n", "Total Simulation Ticks:",
           (unsigned long long)metrics.cpu.getTotalSimulationTicks());
    printf("%-30s %.2f%%\n",
           "CPU Utilization:", metrics.cpu.getCpuUtilization());
    printf("%-30s %g processes/tick\n",
           "Throughput:", metrics.cpu.getThroughput());
    printf("%-30s %.2f ticks\n",
           "Avg Waiting Time:", metrics.cpu.getAvgWaitingTime());
    printf("%-30s %.2f ticks\n",
           "Avg Turnaround Time:", metrics.cpu.getAvgTurnaroundTime());
    printf("%-30s %.2f ticks\n",
           "Avg Response Time:", metrics.cpu.getAvgResponseTime());

    printHeader("SIMULATION RESULTS: MEMORY METRICS (MGLRU)");
    printf("%-30s %zu\n",
           "Total Memory Accesses:", metrics.memory.getTotalAccesses());
    printf("%-30s %zu\n",
           "Total Page Faults:", metrics.memory.getTotalPageFaults());
    printf("%-30s %.4f%%\n",
           "Page Fault Rate:", metrics.memory.getPageFaultRate() * 100.0);

    printf("\n--- Top 5 Processes by Page Faults ---\n");

    auto faultMap = metrics.memory.getPerProcessFaultMap();

    std::vector<std::pair<int, int>> sortedFaults(faultMap.begin(),
                                                  faultMap.end());

    std::sort(sortedFaults.begin(), sortedFaults.end(),
              [](const auto &a, const auto &b) { return a.second > b.second; });

    int count = 0;
    for (const auto &[pid, faults] : sortedFaults) {
      printf("PID %-5d: %d faults\n", pid, faults);

      if (++count >= 5) break;
    }
    std::cout << "======================================================\n";

  } catch (const std::exception &e) {
    fprintf(stderr, "\n[CRITICAL ERROR] %s\n", e.what());
    return 1;
  }

  return 0;
}
