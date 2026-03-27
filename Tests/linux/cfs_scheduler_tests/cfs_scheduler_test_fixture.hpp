#pragma once

#include <gtest/gtest.h>

#include <architecture_config.hpp>
#include <linux_scheduler.hpp>
#include <memory>
#include <metrics.hpp>
#include <process.hpp>

using namespace os_simulation_architecture;
using namespace os_simulation_linux_scheduler;
using namespace os_simulation_metrics;
using namespace os_simulation_process;

class CfsSchedulerTest : public ::testing::Test {
 protected:
  CpuMetrics cpuMetrics;
  std::unique_ptr<LinuxCfsScheduler> scheduler;

  void SetUp() override { scheduler = std::make_unique<LinuxCfsScheduler>(); }
};
