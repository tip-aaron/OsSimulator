#include "cfs_scheduler_test_fixture.hpp"

TEST_F(CfsSchedulerTest, HigherPriorityProcessRunsFirst) {
  Process process(1, 5, 2, 2);

  scheduler->addProcess(process);

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 2);

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 2);

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 1);
  EXPECT_EQ(scheduler->getProcess(1).getStartTime(), 2);

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 0);
  EXPECT_TRUE(scheduler->getProcess(1).isFinished());
  EXPECT_EQ(scheduler->getProcess(1).getCompletionTime(), 4);
}
