#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "memory_api.hpp"
#include "process.hpp"

namespace os_simulation_parser {
enum class WorkloadType {
  INTERACTIVE,
  BACKGROUND,
  MIXED_INTERACTIVE_BACKGROUND
};

std::string to_string(const WorkloadType &workloadType);

struct ProcessWorkload {
  os_simulation_process::Process mProcess;
  std::filesystem::path mTraceFilePath;
};

class WorkloadParser {
 public:
  explicit WorkloadParser(const std::string &projectRoot);

  [[nodiscard]] std::vector<ProcessWorkload> parse(
      const WorkloadType &workloadType) const;

  [[nodiscard]] std::vector<os_simulation_memory::TraceAccess> parseTraceFile(
      const std::filesystem::path &traceFile) const;

 private:
  std::filesystem::path mProjectRoot;

  void getTargetDirectories(const WorkloadType &workloadType,
                            std::filesystem::path &outCsvPath,
                            std::filesystem::path &outTracesDir) const;

  [[nodiscard]] ProcessWorkload parseCsvLine(
      const std::string &line, const std::filesystem::path &tracesDir) const;
};

}  // namespace os_simulation_parser
