#include "cfs_scheduler_test_fixture.hpp"

TEST_F(CfsSchedulerTest, SamePrioritySharesCpuFairly) {
  Process process1(1, 5, 2, 0);
  Process process2(2, 5, 2, 0);

  scheduler->addProcess(process1);
  scheduler->addProcess(process2);

  tickEngine(scheduler);
  tickEngine(scheduler);
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 1);
  EXPECT_EQ(scheduler->getProcess(2).getRemainingTime(), 1);

  tickEngine(scheduler);
  tickEngine(scheduler);
  EXPECT_TRUE(scheduler->isFinished());
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 0);
  EXPECT_EQ(scheduler->getProcess(2).getRemainingTime(), 0);
}