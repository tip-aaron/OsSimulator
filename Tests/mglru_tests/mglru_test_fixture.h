#pragma once

#include <gtest/gtest.h>

#include <architecture_config.hpp>
#include <memory>
#include <metrics.hpp>
#include <mglru_memory.hpp>

using namespace os_simulation_architecture;
using namespace os_simulation_memory;
using namespace os_simulation_metrics;

class MglruTest : public ::testing::Test {
 protected:
  MemoryMetrics memoryMetrics;
  std::unique_ptr<MglruMemoryManager> manager;

  void SetUp() override {
    manager = std::make_unique<MglruMemoryManager>(memoryMetrics);
  }
};
