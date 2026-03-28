#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <workload_parser.hpp>

namespace fs = std::filesystem;

using namespace os_simulation_parser;

class WorkloadParserTest : public ::testing::Test {
 protected:
  fs::path tempRoot;

  void SetUp() override {
    tempRoot = fs::temp_directory_path() / "os_simulation_test_root";

    fs::create_directories(tempRoot);
  }

  void TearDown() override {
    if (fs::exists(tempRoot)) {
      fs::remove_all(tempRoot);
    }
  }

  void createMockCsv(const WorkloadType &type, const std::string &content) {
    fs::path dir = tempRoot / "workloads" / to_string(type);

    fs::create_directories(dir / "traces");

    std::ofstream file(dir / "processes.csv");

    file << content;
    file.close();
  }

  void createMockTraceFile(const WorkloadType &type, int processId) {
    fs::path tracePath = tempRoot / "workloads" / to_string(type) / "traces" /
                         ("process" + std::to_string(processId) + ".ref");
    std::ofstream file(tracePath);
    file << "dummy trace data";  // Just needs to exist
    file.close();
  }
};
