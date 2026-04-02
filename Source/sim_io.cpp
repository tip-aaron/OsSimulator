#include <cassert>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <ostream>
#include <sim_io.hpp>
#include <stdexcept>
#include <string>
#include <string_view>
#include <task.hpp>
#include <vector>

using namespace std;
using namespace os_simulator;

namespace {
// Helper to extract the next delimited token from a string_view
std::string_view next_token(std::string_view &sv, char delimiter) {
  size_t pos = sv.find(delimiter);
  std::string_view token = sv.substr(0, pos);
  if (pos != std::string_view::npos) {
    sv.remove_prefix(pos + 1);
  } else {
    sv = {};
  }
  return token;
}

// Helper to extract the next whitespace-delimited token
std::string_view next_ws_token(std::string_view &sv) {
  sv.remove_prefix(std::min(sv.find_first_not_of(" \t"), sv.size()));
  size_t pos = sv.find_first_of(" \t");
  std::string_view token = sv.substr(0, pos);
  if (pos != std::string_view::npos) {
    sv.remove_prefix(pos);
  } else {
    sv = {};
  }
  return token;
}

// High-performance integer parser using std::from_chars
template <typename T>
T parse_int(std::string_view sv, int base = 10) {
  T value = 0;

  // C++17 compatible "starts_with" check for 0x / 0X
  if (base == 16 && sv.size() >= 2 && sv[0] == '0' &&
      (sv[1] == 'x' || sv[1] == 'X')) {
    sv.remove_prefix(2);
  }

  auto [ptr, ec] =
      std::from_chars(sv.data(), sv.data() + sv.size(), value, base);
  if (ec != std::errc()) {
    throw std::runtime_error("Failed to parse integer: " + std::string(sv));
  }
  return value;
}

// Reusable file reader that processes files in chunks to avoid large
// allocations
template <typename LineProcessor>
void processFileByChunks(const std::filesystem::path &path,
                         LineProcessor processor) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + path.string());
  }

  constexpr size_t BUFFER_SIZE = 64 * 1024;  // 64KB chunks
  std::vector<char> buffer(BUFFER_SIZE);
  size_t leftover_size = 0;

  while (
      file.read(buffer.data() + leftover_size, buffer.size() - leftover_size) ||
      file.gcount() > 0) {
    size_t bytesRead = file.gcount();
    size_t total_size = leftover_size + bytesRead;
    std::string_view chunk(buffer.data(), total_size);

    size_t last_newline = chunk.find_last_of('\n');

    // If the chunk doesn't have a newline, expand buffer and read more
    if (last_newline == std::string_view::npos) {
      leftover_size = total_size;
      if (leftover_size == buffer.size()) {
        buffer.resize(buffer.size() * 2);
      }
      continue;
    }

    std::string_view process_chunk = chunk.substr(0, last_newline);

    size_t pos = 0;
    while (pos < process_chunk.size()) {
      size_t nl_pos = process_chunk.find('\n', pos);
      if (nl_pos == std::string_view::npos) nl_pos = process_chunk.size();

      std::string_view line = process_chunk.substr(pos, nl_pos - pos);
      if (!line.empty() && line.back() == '\r') {
        line.remove_suffix(1);
      }

      if (!line.empty()) processor(line);

      pos = nl_pos + 1;
    }

    // Move leftover fragment to the front of the buffer for the next read
    leftover_size = total_size - (last_newline + 1);
    std::memmove(buffer.data(), buffer.data() + last_newline + 1,
                 leftover_size);
  }

  // Process any remaining bytes that didn't end in a newline
  if (leftover_size > 0) {
    std::string_view line(buffer.data(), leftover_size);
    if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
    if (!line.empty()) processor(line);
  }
}
}  // namespace

WorkloadParser::WorkloadParser(const string &relativeWorkloadRootPath) {
  mWorkloadPath = filesystem::absolute(relativeWorkloadRootPath);
  assert(filesystem::exists(mWorkloadPath));
}

void WorkloadParser::getTargetDirectories(
    const WorkloadType workloadType, filesystem::path &rTaskDataPath,
    filesystem::path &rTracesDataPath) const {
  filesystem::path workloadDir =
      mWorkloadPath / "workloads" / to_string(workloadType);
  rTaskDataPath = workloadDir / "processes.csv";
  rTracesDataPath = workloadDir / "traces";

  cout << rTaskDataPath << endl;

  if (!filesystem::exists(rTaskDataPath)) {
    throw runtime_error("Task file not found! Looked at: " +
                        filesystem::absolute(rTaskDataPath).string());
  }

  if (!filesystem::exists(rTracesDataPath)) {
    throw runtime_error("Traces folder not found! Looked at: " +
                        filesystem::absolute(rTracesDataPath).string());
  }
}

vector<unique_ptr<MemoryPageAccess>> WorkloadParser::parseTraceFile(
    const filesystem::path &tracesDataPath) const {
  if (!filesystem::exists(tracesDataPath)) {
    throw runtime_error("Trace file not found: " + tracesDataPath.string());
  }

  cout << "\tParsing trace file..." << endl;

  vector<unique_ptr<MemoryPageAccess>> memoryPageAccesses;
  uint64_t cumulativeTaskTicks = 0;

  auto lineProcessor = [&](std::string_view line) {
    std::string_view accessTypeStr = next_ws_token(line);
    std::string_view memoryHexAddressStr = next_ws_token(line);
    std::string_view nonMemoryInstructionTicksStr = next_ws_token(line);

    if (accessTypeStr.empty() || memoryHexAddressStr.empty() ||
        nonMemoryInstructionTicksStr.empty()) {
      return;
    }

    // Convert string_view to string for from_string if it doesn't natively
    // support string_view
    MemoryPageAccessType memoryPageAccessType =
        from_string(std::string(accessTypeStr));
    uint64_t vpn = 0;
    int nonMemoryTicks = 0;

    try {
      vpn = parse_int<uint64_t>(memoryHexAddressStr, 16);
    } catch (const exception &) {
      throw runtime_error(
          "Invalid memory address in trace file: " + tracesDataPath.string() +
          ": " + std::string(memoryHexAddressStr));
    }

    try {
      nonMemoryTicks = parse_int<int>(nonMemoryInstructionTicksStr, 10);
    } catch (const exception &) {
      throw runtime_error(
          "Invalid non-memory instruction ticks in trace file: " +
          tracesDataPath.string() + ": " +
          std::string(nonMemoryInstructionTicksStr));
    }

    uint64_t pageVpn = MemoryPageAccess::convertByteToVpn(vpn);
    uint64_t absoluteTickOfThisAccess = cumulativeTaskTicks + nonMemoryTicks;

    if (memoryPageAccesses.empty() ||
        memoryPageAccesses.back()->vpn != pageVpn ||
        memoryPageAccesses.back()->type != memoryPageAccessType) {
      MemoryPageAccess newPageAccess;
      newPageAccess.vpn = pageVpn;
      newPageAccess.type = memoryPageAccessType;
      newPageAccess.accessCount = 1;
      newPageAccess.relativeTick = absoluteTickOfThisAccess;
      newPageAccess.totalTicks = nonMemoryTicks + 1;

      memoryPageAccesses.push_back(
          make_unique<MemoryPageAccess>(newPageAccess));
    } else {
      MemoryPageAccess *prevMemoryPage = memoryPageAccesses.back().get();
      prevMemoryPage->accessCount++;
      prevMemoryPage->totalTicks += nonMemoryTicks + 1;
    }

    cumulativeTaskTicks += nonMemoryTicks + 1;
  };

  processFileByChunks(tracesDataPath, lineProcessor);

  return memoryPageAccesses;
}

unique_ptr<Task> WorkloadParser::parseCsvLine(
    std::string_view rLine, const filesystem::path &rTracesDataPath) const {
  std::string_view idStr = next_token(rLine, ',');
  std::string_view priorityStr = next_token(rLine, ',');
  std::string_view burstTimeStr = next_token(rLine, ',');
  std::string_view arrivalTimeStr = next_token(rLine, ',');

  cout << "Parsing task#" << idStr << "" << endl;

  int id = parse_int<int>(idStr);
  int priority = parse_int<int>(priorityStr);
  int burstTime = parse_int<int>(burstTimeStr);
  int arrivalTime = parse_int<int>(arrivalTimeStr);

  vector<unique_ptr<MemoryPageAccess>> memoryPageAccesses = parseTraceFile(
      rTracesDataPath / ("process_" + std::string(idStr) + ".ref"));

  return make_unique<Task>(id, "TASK_" + std::string(idStr), priority,
                           arrivalTime, burstTime,
                           std::move(memoryPageAccesses));
}

vector<unique_ptr<Task>> WorkloadParser::parse(
    WorkloadType workloadType) const {
  filesystem::path taskDataPath, tracesDataPath;
  getTargetDirectories(workloadType, taskDataPath, tracesDataPath);

  vector<unique_ptr<Task>> tasks;
  bool isFirstLine = true;

  auto lineProcessor = [&](std::string_view line) {
    if (isFirstLine) {
      isFirstLine = false;  // Skip CSV header

      return;
    }
    try {
      tasks.push_back(parseCsvLine(line, tracesDataPath));
    } catch (const exception &e) {
      fprintf(stderr, "%s\n", e.what());
    }
  };

  processFileByChunks(taskDataPath, lineProcessor);

  return tasks;
}