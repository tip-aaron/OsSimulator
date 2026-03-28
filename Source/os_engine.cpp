#include "os_engine.hpp"

#include <thread>

os_simulation_engine::OsSimulationEngine::OsSimulationEngine(
    std::shared_ptr<os_simulation_scheduler::IScheduler> scheduler,
    std::shared_ptr<os_simulation_memory::IMemoryManager> memoryManager)
    : mScheduler(std::move(scheduler)),
      mMemoryManager(std::move(memoryManager)) {}

void os_simulation_engine::OsSimulationEngine::runSimulation() {
  std::atomic<bool> isSimulating{true};
  auto simStartTime = std::chrono::steady_clock::now();
  size_t totalProcesses = mProcessTraces.size();

  std::thread loaderThread([&]() {
    const char spinner[] = {'|', '/', '-', '\\'};
    int spinIdx = 0;

    while (isSimulating) {
      auto now = std::chrono::steady_clock::now();
      auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                           now - simStartTime)
                           .count();
      double elapsedSec = elapsedMs / 1000.0;
      size_t completedCount = 0;

      for (const auto &[pid, traces] : mProcessTraces) {
        const auto &proc = mScheduler->getProcess(pid);
        if (proc != nullptr && proc->getCompletionTime() > 0) {
          completedCount++;
        }
      }

      float percent =
          (totalProcesses > 0)
              ? (static_cast<float>(completedCount) / totalProcesses) * 100.0f
              : 0.0f;

      std::cout << "\r[" << spinner[spinIdx % 4]
                << "] Simulating: " << completedCount << " / " << totalProcesses
                << " finished (" << std::fixed << std::setprecision(1)
                << percent << "%) - " << std::fixed << std::setprecision(1)
                << elapsedSec << "s elapsed   " << std::flush;

      spinIdx++;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "\r" << std::string(90, ' ') << "\r" << std::flush;
  });

  while (!mScheduler->isFinished()) {
    auto *pRunningProcess = mScheduler->getNextProcessToRun();

    if (pRunningProcess != nullptr) {
      int pid = pRunningProcess->getId();
      os_simulation_memory::TraceAccess nextTraceAccess =
          getNextTraceForProcess(pid);

      bool isHit = mMemoryManager->accessAddress(
          pid, nextTraceAccess.mVirtualAddress, nextTraceAccess.mAccessType);

      if (!isHit) {
        mMemoryManager->handlePageFault(pRunningProcess->getId(),
                                        nextTraceAccess.mVirtualAddress,
                                        nextTraceAccess.mAccessType);
        pRunningProcess->block(
            os_simulation_architecture::BACKING_STORE_LATENCY_MS);

        mTraceAccessIndices[pid]--;
      } else {
        mScheduler->executeProcess(pRunningProcess);
      }
    }

    mScheduler->addTick();
  }

  isSimulating = false;
  loaderThread.join();

  auto simEndTime = std::chrono::steady_clock::now();
  double totalSec = std::chrono::duration_cast<std::chrono::milliseconds>(
                        simEndTime - simStartTime)
                        .count() /
                    1000.0;

  std::cout << "Engine finished crunching in " << std::fixed
            << std::setprecision(2) << totalSec << " seconds.\n";
}

void os_simulation_engine::OsSimulationEngine::loadWorkload(
    const std::vector<os_simulation_parser::ProcessWorkload> &rWorkloads,
    const os_simulation_parser::WorkloadParser &rParser) {
  for (const auto &rWorkload : rWorkloads) {
    mScheduler->addProcess(rWorkload.mProcess);

    std::vector<os_simulation_memory::TraceAccess> traceAccessVec =
        rParser.parseTraceFile(rWorkload.mTraceFilePath);
    int pid = rWorkload.mProcess.getId();
    mProcessTraces[pid] = std::move(traceAccessVec);

    mTraceAccessIndices[pid] = 0;
  }
}

os_simulation_memory::TraceAccess
os_simulation_engine::OsSimulationEngine::getNextTraceForProcess(
    int processId) {
  auto traceIt = mProcessTraces.find(processId);
  auto indexIt = mTraceAccessIndices.find(processId);

  if (traceIt == mProcessTraces.end() || indexIt == mTraceAccessIndices.end()) {
    throw std::runtime_error("No trace file loaded for Process ID " +
                             std::to_string(processId));
  }

  const auto &traces = traceIt->second;
  size_t currentLineIndex = indexIt->second;

  if (currentLineIndex >= traces.size()) {
    std::cerr << "Process " << processId << " outran its trace file!\n";

    return {0x0, os_simulation_memory::MemoryAccessType::READ};
  }

  os_simulation_memory::TraceAccess currentTraceAccess =
      traces[currentLineIndex];

  mTraceAccessIndices[processId]++;

  return currentTraceAccess;
}
