#include "mglru_test_fixture.hpp"

TEST_F(MglruTest, BasicEvictionWhenMemoryFull) {
  for (uint32_t i = 0; i < PHYSICAL_FRAME_COUNT; ++i) {
    manager->handlePageFault(1, i * PAGE_SIZE_BYTES, MemoryAccessType::READ);
  }

  uint64_t oldestAddress = 0;
  uint64_t newestAddress = (PHYSICAL_FRAME_COUNT - 1) * PAGE_SIZE_BYTES;

  EXPECT_TRUE(manager->accessAddress(1, oldestAddress, MemoryAccessType::READ));
  EXPECT_TRUE(manager->accessAddress(1, newestAddress, MemoryAccessType::READ));

  uint64_t overflowingAddress = PHYSICAL_FRAME_COUNT * PAGE_SIZE_BYTES;

  manager->handlePageFault(1, overflowingAddress, MemoryAccessType::READ);

  EXPECT_FALSE(
      manager->accessAddress(1, oldestAddress, MemoryAccessType::READ));
  EXPECT_TRUE(
      manager->accessAddress(1, overflowingAddress, MemoryAccessType::READ));
}

TEST_F(MglruTest, EvictionRespectsReferencedPages) {
  for (uint32_t i = 0; i < PHYSICAL_FRAME_COUNT; ++i) {
    manager->handlePageFault(1, i * PAGE_SIZE_BYTES, MemoryAccessType::READ);
  }

  uint64_t savedAddress = 0;
  uint64_t doomedAddress = 1 * PAGE_SIZE_BYTES;

  EXPECT_TRUE(manager->accessAddress(1, savedAddress, MemoryAccessType::READ));

  manager->ageGenerations();

  uint64_t overflowingAddress = PHYSICAL_FRAME_COUNT * PAGE_SIZE_BYTES;

  manager->handlePageFault(1, overflowingAddress, MemoryAccessType::READ);

  EXPECT_FALSE(
      manager->accessAddress(1, doomedAddress, MemoryAccessType::READ));
  EXPECT_TRUE(manager->accessAddress(1, savedAddress, MemoryAccessType::READ));
  EXPECT_TRUE(
      manager->accessAddress(1, overflowingAddress, MemoryAccessType::READ));
}

TEST_F(MglruTest, EvictsFromOldestGeneration) {
  // 1. Fill up all physical memory to the brim
  for (uint32_t i = 0; i < PHYSICAL_FRAME_COUNT; ++i) {
    manager->handlePageFault(1, i * PAGE_SIZE_BYTES, MemoryAccessType::READ);
  }

  for (uint32_t i = 0; i < manager->NUM_GENERATIONS; ++i) {
    manager->ageGenerations();
  }

  uint64_t savedAddress = 0;
  uint64_t doomedAddress = 1 * PAGE_SIZE_BYTES;

  EXPECT_TRUE(manager->accessAddress(1, savedAddress, MemoryAccessType::READ));
  manager->ageGenerations();

  uint64_t overflowingAddress = PHYSICAL_FRAME_COUNT * PAGE_SIZE_BYTES;

  manager->handlePageFault(1, overflowingAddress, MemoryAccessType::READ);

  EXPECT_TRUE(
      manager->accessAddress(1, overflowingAddress, MemoryAccessType::READ));
  EXPECT_TRUE(manager->accessAddress(1, savedAddress, MemoryAccessType::READ));
  EXPECT_FALSE(
      manager->accessAddress(1, doomedAddress, MemoryAccessType::READ));
}