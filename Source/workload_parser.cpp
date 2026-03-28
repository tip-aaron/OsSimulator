#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <workload_parser.hpp>

namespace fs = std::filesystem;

namespace os_simulation_parser {
std::string to_string(const WorkloadType &workloadType) {
  switch (workloadType) {
    case WorkloadType::INTERACTIVE:
      return "interactive";
    case WorkloadType::BACKGROUND:
      return "background";
    case WorkloadType::MIXED_INTERACTIVE_BACKGROUND:
      return "mixed_interactive_and_background";
    default:
      throw std::invalid_argument("Invalid workload type");
  };
}

WorkloadParser::WorkloadParser(const std::string &projectRoot)
    : mProjectRoot(projectRoot) {
  if (!fs::exists(mProjectRoot) || !fs::is_directory(mProjectRoot)) {
    std::cerr << "Attempted to resolve path: " << fs::absolute(mProjectRoot)
              << "\n";

    throw std::invalid_argument(
        "Project root path does not exist or is not a directory");
  }
}

void WorkloadParser::getTargetDirectories(const WorkloadType &workloadType,
                                          fs::path &outCsvPath,
                                          fs::path &outTracesDir) const {
  fs::path workloadDir = mProjectRoot / "workloads" / to_string(workloadType);
  outCsvPath = workloadDir / "processes.csv";
  outTracesDir = workloadDir / "traces";

  if (!fs::exists(outCsvPath)) {
    throw std::runtime_error("CSV file not found at: " + outCsvPath.string());
  }
}

ProcessWorkload WorkloadParser::parseCsvLine(const std::string &line,
                                             const fs::path &tracesDir) const {
  std::stringstream ss(line);
  std::string idStr, priorityStr, burstStr, arrivalStr;

  std::getline(ss, idStr, ',');
  std::getline(ss, priorityStr, ',');
  std::getline(ss, burstStr, ',');
  std::getline(ss, arrivalStr, ',');

  try {
    int id = std::stoi(idStr);
    int priority = std::stoi(priorityStr);
    int burst = std::stoi(burstStr);
    int arrival = std::stoi(arrivalStr);

    // Construct the expected trace file path
    fs::path traceFile = tracesDir / ("process_" + idStr + ".ref");

    if (!fs::exists(traceFile)) {
      std::cerr << "Warning: Missing trace file for Process ID " << id << " at "
                << traceFile << "\n";
    }

    os_simulation_process::Process process(id, priority, burst, arrival);

    return ProcessWorkload{process, traceFile};
  } catch (const std::exception &e) {
    throw std::runtime_error("Error parsing CSV line: " + line + " - " +
                             e.what());
  }
}

std::vector<ProcessWorkload> WorkloadParser::parse(
    const WorkloadType &workloadType) const {
  fs::path csvFile, tracesDir;

  getTargetDirectories(workloadType, csvFile, tracesDir);

  std::ifstream file(csvFile);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open CSV file: " + csvFile.string());
  }

  std::vector<ProcessWorkload> workloads;
  std::string line;

  std::getline(file, line);

  while (std::getline(file, line)) {
    if (line.empty()) {
      continue;
    }

    try {
      ProcessWorkload workload = parseCsvLine(line, tracesDir);

      workloads.push_back(workload);
    } catch (const std::exception &e) {
      std::cerr << e.what() << "\n";
    }
  }

  return workloads;
}

std::vector<os_simulation_memory::TraceAccess> WorkloadParser::parseTraceFile(
    const fs::path &traceFile) const {
  std::vector<os_simulation_memory::TraceAccess> traceAccessVec;

  if (!fs::exists(traceFile)) {
    std::cerr << "Trace file not found: " << traceFile << "\n";

    return traceAccessVec;
  }

  std::ifstream file(traceFile);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open trace file: " +
                             traceFile.string());
  }

  std::string typeStr;
  std::string hexStr;

  while (file >> typeStr >> hexStr) {
    os_simulation_memory::TraceAccess traceAccess;

    if (typeStr == "W" || typeStr == "w") {
      traceAccess.mAccessType = os_simulation_memory::MemoryAccessType::WRITE;
    } else if (typeStr == "R" || typeStr == "r") {
      traceAccess.mAccessType = os_simulation_memory::MemoryAccessType::READ;
    } else {
      throw std::runtime_error("Invalid access type in trace file " +
                               traceFile.string() +
                               ": expected R or W but got '" + typeStr + "'");
    }

    try {
      traceAccess.mVirtualAddress =
          static_cast<uint32_t>(std::stoull(hexStr, nullptr, 0));
    } catch (const std::exception &e) {
      throw std::runtime_error("Invalid hex address in trace file " +
                               traceFile.string() + ": " + hexStr);
    }

    traceAccessVec.push_back(traceAccess);
  }

  return traceAccessVec;
}

}  // namespace os_simulation_parser