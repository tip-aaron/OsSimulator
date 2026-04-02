#include <algorithm>
#include <factory.hpp>
#include <iomanip>
#include <iostream>
#include <memory.hpp>
#include <memory>
#include <scheduler.hpp>
#include <sim_engine.hpp>
#include <sim_io.hpp>
#include <sim_metrics.hpp>
#include <string>
#include <task.hpp>
#include <vector>

#include "utils/threading.hpp"

using namespace std;
using namespace os_simulator;

void printTableSeparator(int width, char fill = '-') {
  cout << "+" << setfill(fill) << setw(width - 2) << "" << "+" << setfill(' ')
       << endl;
}

int main(int argc, char *argv[]) {
  string workloadRoot = (argc > 1) ? argv[1] : ".";

  cout << "Loading workload from: " << workloadRoot << "..." << endl;
  WorkloadParser parser(workloadRoot);

  vector<unique_ptr<Task>> tasks =
      parser.parse(WorkloadType::MIXED_INTERACTIVE_BACKGROUND);
  size_t totalTasks = tasks.size();

  if (totalTasks == 0) {
    cerr << "Warning: No tasks loaded. Exiting." << endl;
    return 0;
  }

  cout << "Successfully loaded " << totalTasks << " tasks.\n" << endl;

  shared_ptr<IScheduler> scheduler = createScheduler();
  shared_ptr<IMemoryManager> memoryManager = createMemoryManager();

  SimulationEngine engine(scheduler, memoryManager, std::move(tasks));

  ConsoleSpinnerReporter reporter([&engine]() -> size_t {
    return engine.getMetrics()->cpu.getCompletedProcessCount();
  });

  reporter.start(totalTasks);
  engine.run();
  reporter.stop();

  const OsSimulationMetrics *metrics = engine.getMetrics();
  const CpuMetrics &cpu = metrics->cpu;
  const MemoryMetrics &mem = metrics->memory;

  const int TABLE_WIDTH = 55;

  cout << "\n\n";
  printTableSeparator(TABLE_WIDTH, '=');
  cout << "| " << left << setw(TABLE_WIDTH - 4) << "OS SIMULATION RESULTS"
       << " |" << endl;
  printTableSeparator(TABLE_WIDTH, '=');

  cout << "| " << left << setw(TABLE_WIDTH - 4) << "CPU & SCHEDULING METRICS"
       << " |" << endl;
  printTableSeparator(TABLE_WIDTH);

  cout << "| " << left << setw(30) << "Total Simulation Ticks"
       << "| " << right << setw(19) << cpu.getTotalSimulationTicks() << " |"
       << endl;

  cout << "| " << left << setw(30) << "Total Busy Ticks"
       << "| " << right << setw(19) << cpu.getTotalBusyTicks() << " |" << endl;

  cout << "| " << left << setw(30) << "Total Idle Ticks"
       << "| " << right << setw(19) << cpu.getTotalIdleTicks() << " |" << endl;

  cout << "| " << left << setw(30) << "Context Switch Count"
       << "| " << right << setw(19) << cpu.getContextSwitchCounts() << " |"
       << endl;

  cout << "| " << left << setw(30) << "Context Switch Ticks"
       << "| " << right << setw(19) << cpu.getTotalContextSwitchTicks() << " |"
       << endl;

  printTableSeparator(TABLE_WIDTH);

  cout << "| " << left << setw(30) << "CPU Utilization"
       << "| " << right << fixed << setprecision(2) << setw(18)
       << cpu.getCpuUtilization() << "% |" << endl;

  cout << "| " << left << setw(30) << "Throughput (Tasks/Tick)"
       << "| " << right << fixed << setprecision(6) << setw(19)
       << cpu.getThroughput() << " |" << endl;

  cout << "| " << left << setw(30) << "Avg Waiting Time"
       << "| " << right << fixed << setprecision(2) << setw(19)
       << cpu.getAvgWaitingTime() << " |" << endl;

  cout << "| " << left << setw(30) << "Avg Turnaround Time"
       << "| " << right << fixed << setprecision(2) << setw(19)
       << cpu.getAvgTurnaroundTime() << " |" << endl;

  cout << "| " << left << setw(30) << "Avg Response Time"
       << "| " << right << fixed << setprecision(2) << setw(19)
       << cpu.getAvgResponseTime() << " |" << endl;

  printTableSeparator(TABLE_WIDTH, '=');

  double jainIndex = cpu.getJainsFairnessIndex();
  string fairnessLabel;
  if (jainIndex >= 0.95)
    fairnessLabel = "(Excellent)";
  else if (jainIndex >= 0.8)
    fairnessLabel = "(Good)";
  else if (jainIndex >= 0.6)
    fairnessLabel = "(Fair)";
  else
    fairnessLabel = "(Poor/Biased)";

  cout << "| " << left << setw(20) << "Jain Fairness Index" << right << setw(10)
       << fairnessLabel << "| " << right << fixed << setprecision(4) << setw(19)
       << jainIndex << " |" << endl;

  printTableSeparator(TABLE_WIDTH, '=');

  // --- MEMORY METRICS TABLE ---
  cout << "| " << left << setw(TABLE_WIDTH - 4) << "MEMORY & PAGING METRICS"
       << " |" << endl;
  printTableSeparator(TABLE_WIDTH);

  cout << "| " << left << setw(30) << "Total Memory Accesses"
       << "| " << right << setw(19) << mem.getTotalAccesses() << " |" << endl;

  cout << "| " << left << setw(30) << "Total Page Faults"
       << "| " << right << setw(19) << mem.getTotalPageFaults() << " |" << endl;

  cout << "| " << left << setw(30) << "Page Fault Rate"
       << "| " << right << fixed << setprecision(4) << setw(18)
       << (mem.getPageFaultRate() * 100.0) << "% |" << endl;

  printTableSeparator(TABLE_WIDTH, '=');
  cout << endl;

  // --- TOP 10 TASK FAULTS ---
  cout << "| " << left << setw(TABLE_WIDTH - 4) << "TOP 10 TASK PAGE FAULTS"
       << " |" << endl;
  printTableSeparator(TABLE_WIDTH);
  cout << "| " << left << setw(15) << "Task ID"
       << "| " << right << setw(34) << "Fault Count" << " |" << endl;
  printTableSeparator(TABLE_WIDTH);

  // 1. Get the map and copy to a vector for sorting
  auto faultMap = mem.getPerTaskFaultMap();
  vector<pair<uint16_t, uint32_t>> sortedFaults(faultMap.begin(),
                                                faultMap.end());

  // 2. Sort descending by fault count (second element of the pair)
  sort(sortedFaults.begin(), sortedFaults.end(),
       [](const auto &a, const auto &b) { return a.second > b.second; });

  // 3. Print the top 10
  int count = 0;
  for (const auto &[taskId, faultCount] : sortedFaults) {
    if (count++ >= 10) break;
    cout << "| " << left << "ID: " << setw(11) << taskId << "| " << right
         << setw(34) << faultCount << " |" << endl;
  }

  printTableSeparator(TABLE_WIDTH, '=');
  cout << endl;

  return EXIT_SUCCESS;
}