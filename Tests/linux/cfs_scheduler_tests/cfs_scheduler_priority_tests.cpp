#include "cfs_scheduler_test_fixture.hpp"

TEST_F(CfsSchedulerTest, SamePrioritySharesCpuFairly) {
  Process process1(1, 5, 2, 0);
  Process process2(2, 5, 2, 0);

  scheduler->addProcess(process1);
  scheduler->addProcess(process2);

  scheduler->addTick();
  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 1);
  EXPECT_EQ(scheduler->getProcess(2).getRemainingTime(), 1);

  scheduler->addTick();
  scheduler->addTick();
  EXPECT_TRUE(scheduler->isFinished());
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 0);
  EXPECT_EQ(scheduler->getProcess(2).getRemainingTime(), 0);
}

TEST_F(CfsSchedulerTest, HighPriorityRunsBeforeLowPriority) {
  Process process1(1, 1, 3, 0);
  Process process2(2, 10, 3, 0);

  scheduler->addProcess(process1);
  scheduler->addProcess(process2);

  scheduler->addTick();
  scheduler->addTick();
  scheduler->addTick();

  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 0);
  EXPECT_TRUE(scheduler->getProcess(1).isFinished());

  EXPECT_EQ(scheduler->getProcess(2).getRemainingTime(), 3);
  EXPECT_FALSE(scheduler->isFinished());

  scheduler->addTick();
  scheduler->addTick();
  scheduler->addTick();

  EXPECT_TRUE(scheduler->isFinished());
  EXPECT_EQ(scheduler->getProcess(2).getRemainingTime(), 0);
}
