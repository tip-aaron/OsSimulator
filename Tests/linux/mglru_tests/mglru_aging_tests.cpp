#include "mglru_test_fixture.hpp"

TEST_F(MglruTest, BasicEvictionWhenMemoryFull) {
  for (uint32_t i = 0; i < PHYSICAL_FRAME_COUNT; ++i) {
    manager->handlePageFault(i * PAGE_SIZE_BYTES);
  }

  uint64_t oldestAddress = 0;
  uint64_t newestAddress = (PHYSICAL_FRAME_COUNT - 1) * PAGE_SIZE_BYTES;

  EXPECT_TRUE(manager->accessAddress(oldestAddress));
  EXPECT_TRUE(manager->accessAddress(newestAddress));

  uint64_t overflowingAddress = PHYSICAL_FRAME_COUNT * PAGE_SIZE_BYTES;

  manager->handlePageFault(overflowingAddress);

  EXPECT_FALSE(manager->accessAddress(oldestAddress));
  EXPECT_TRUE(manager->accessAddress(overflowingAddress));
}
