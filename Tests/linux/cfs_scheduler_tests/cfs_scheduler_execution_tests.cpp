#include "cfs_scheduler_test_fixture.hpp"

TEST_F(CfsSchedulerTest, SingleProcessExecution) {
  Process process(1, 5, 3, 0);

  scheduler->addProcess(process);

  EXPECT_FALSE(scheduler->isFinished());

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 2);
  EXPECT_EQ(scheduler->getProcess(1).getStartTime(), 0);

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 1);

  scheduler->addTick();
  EXPECT_EQ(scheduler->getProcess(1).getRemainingTime(), 0);
  EXPECT_TRUE(scheduler->isFinished());
  EXPECT_EQ(scheduler->getProcess(1).getCompletionTime(), 3);
}
