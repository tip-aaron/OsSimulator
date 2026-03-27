#include "mglru_test_fixture.h"

TEST_F(MglruTest, InitialAccessCausesPageFault) {
  uint64_t address = 0x1000;

  EXPECT_FALSE(manager->accessAddress(address));
  EXPECT_EQ(memoryMetrics.getTotalPageFaults(), 1);
  EXPECT_EQ(memoryMetrics.getTotalAccesses(), 1);
}

TEST_F(MglruTest, HandlePageFaultBringsPageIntoMemory) {
  uint64_t address = 0x1000;

  EXPECT_FALSE(manager->accessAddress(address));
  manager->handlePageFault(address);
  EXPECT_TRUE(manager->accessAddress(address));

  EXPECT_EQ(memoryMetrics.getTotalPageFaults(), 1);
  EXPECT_EQ(memoryMetrics.getTotalAccesses(), 2);
}
