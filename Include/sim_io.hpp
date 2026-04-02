#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "task.hpp"

using namespace std;

namespace os_simulator {
enum class WorkloadType {
  INTERACTIVE,
  BACKGROUND,
  MIXED_INTERACTIVE_BACKGROUND
};

inline string to_string(const WorkloadType workloadType) {
  switch (workloadType) {
    case WorkloadType::INTERACTIVE:
      return "interactive";
    case WorkloadType::BACKGROUND:
      return "background";
    case WorkloadType::MIXED_INTERACTIVE_BACKGROUND:
      return "mixed_interactive_and_background";
  }
}

class WorkloadParser {
 public:
  /*
   * @param relativeWorkloadPath relative from current directory where this
   * program is called, extend the path where to look.
   */
  explicit WorkloadParser(const string &relativeWorkloadPath);

  [[nodiscard]] vector<unique_ptr<Task>> parse(
      const WorkloadType workloadType) const;

 private:
  filesystem::path mWorkloadPath;

  void getTargetDirectories(const WorkloadType workloadType,
                            filesystem::path &rTaskDataPath,
                            filesystem::path &rTracesDataPath) const;

  [[nodiscard]] vector<unique_ptr<MemoryPageAccess>> parseTraceFile(
      const filesystem::path &tracesDataPath) const;
  [[nodiscard]] unique_ptr<Task> parseCsvLine(
      std::string_view rLine, const filesystem::path &rTracesDataPath) const;
};
}  // namespace os_simulator
