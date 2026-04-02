#include <gtest/gtest.h>

#include <rbtree.hpp>

using namespace os_simulator;

class RbTreeTestFixture : public ::testing::Test {
 protected:
  RbTree<int> tree;

  void SetUp() override {}

  void TearDown() override {}
};

TEST_F(RbTreeTestFixture, ExtractMinimumOnEmptyTreeThrows) {
  // Expect a runtime error when extracting from an empty tree
  EXPECT_THROW(tree.extractMinimum(), std::runtime_error);
}

TEST_F(RbTreeTestFixture, InsertAndExtractSingleElement) {
  tree.insert(42);

  // Should extract the only element
  EXPECT_EQ(tree.extractMinimum(), 42);

  // Tree should be empty again, triggering the exception
  EXPECT_THROW(tree.extractMinimum(), std::runtime_error);
}

TEST_F(RbTreeTestFixture, InsertMultipleAndExtractInOrder) {
  // Insert values out of order to simulate CFS vruntimes
  tree.insert(50);
  tree.insert(30);
  tree.insert(70);
  tree.insert(20);
  tree.insert(40);
  tree.insert(60);
  tree.insert(80);

  // extractMinimum should always pull the lowest available value (furthest
  // left)
  EXPECT_EQ(tree.extractMinimum(), 20);
  EXPECT_EQ(tree.extractMinimum(), 30);
  EXPECT_EQ(tree.extractMinimum(), 40);
  EXPECT_EQ(tree.extractMinimum(), 50);
  EXPECT_EQ(tree.extractMinimum(), 60);
  EXPECT_EQ(tree.extractMinimum(), 70);
  EXPECT_EQ(tree.extractMinimum(), 80);

  // Verify tree is fully emptied
  EXPECT_THROW(tree.extractMinimum(), std::runtime_error);
}

TEST_F(RbTreeTestFixture, HandleNegativeValues) {
  tree.insert(-10);
  tree.insert(0);
  tree.insert(-50);
  tree.insert(10);

  EXPECT_EQ(tree.extractMinimum(), -50);
  EXPECT_EQ(tree.extractMinimum(), -10);
  EXPECT_EQ(tree.extractMinimum(), 0);
  EXPECT_EQ(tree.extractMinimum(), 10);
}

TEST_F(RbTreeTestFixture, HandleDuplicateValues) {
  tree.insert(15);
  tree.insert(15);
  tree.insert(5);
  tree.insert(5);

  EXPECT_EQ(tree.extractMinimum(), 5);
  EXPECT_EQ(tree.extractMinimum(), 5);
  EXPECT_EQ(tree.extractMinimum(), 15);
  EXPECT_EQ(tree.extractMinimum(), 15);
}

TEST_F(RbTreeTestFixture, LargeDataStressTest) {
  // Insert 1000 descending values
  for (int i = 1000; i > 0; --i) {
    tree.insert(i);
  }

  // Extraction should return them correctly in ascending order (1 to 1000)
  for (int i = 1; i <= 1000; ++i) {
    EXPECT_EQ(tree.extractMinimum(), i);
  }
}

TEST_F(RbTreeTestFixture, RemoveLeafNode) {
  tree.insert(50);
  tree.insert(30);
  tree.insert(70);

  // 30 and 70 are likely leaves depending on color.
  // Removing a leaf is the simplest case, but black leaves trigger rebalancing.
  tree.remove(30);
  tree.remove(70);

  EXPECT_EQ(tree.extractMinimum(), 50);
  EXPECT_THROW(tree.extractMinimum(), std::runtime_error);
}

TEST_F(RbTreeTestFixture, RemoveNodeWithOneChild) {
  tree.insert(50);
  tree.insert(30);
  tree.insert(20);  // 30 now has one left child (20)

  // Removing 30 should bypass it and link 50 directly to 20
  tree.remove(30);

  EXPECT_EQ(tree.extractMinimum(), 20);
  EXPECT_EQ(tree.extractMinimum(), 50);
}

TEST_F(RbTreeTestFixture, RemoveNodeWithTwoChildren) {
  tree.insert(50);
  tree.insert(30);
  tree.insert(70);
  tree.insert(20);
  tree.insert(40);

  // 30 has two children (20 and 40).
  // The tree must find the in-order successor (40), swap it with 30, and then
  // delete.
  tree.remove(30);

  EXPECT_EQ(tree.extractMinimum(), 20);
  EXPECT_EQ(tree.extractMinimum(), 40);
  EXPECT_EQ(tree.extractMinimum(), 50);
  EXPECT_EQ(tree.extractMinimum(), 70);
}

TEST_F(RbTreeTestFixture, RemoveRootNode) {
  tree.insert(50);
  tree.insert(30);
  tree.insert(70);

  // Deleting the root forces the tree to reassign the root pointer
  tree.remove(50);

  EXPECT_EQ(tree.extractMinimum(), 30);
  EXPECT_EQ(tree.extractMinimum(), 70);
  EXPECT_THROW(tree.extractMinimum(), std::runtime_error);
}

TEST_F(RbTreeTestFixture, RemoveNonExistentValueIgnoresOrThrows) {
  tree.insert(50);

  // Depending on your implementation, this should either quietly return or
  // throw. Assuming it does nothing:
  tree.remove(999);

  EXPECT_EQ(tree.extractMinimum(), 50);
}

TEST_F(RbTreeTestFixture, SchedulerStressTest_InterleavedInsertAndRemove) {
  // This mimics exactly what the scheduler is doing:
  // inserting, pulling tasks out to update them, and putting them back.
  tree.insert(100);
  tree.insert(50);
  tree.insert(150);
  tree.insert(25);
  tree.insert(75);
  tree.insert(125);
  tree.insert(175);

  // Yanking nodes out of the middle of the tree
  tree.remove(75);
  tree.remove(150);
  tree.remove(25);

  // Inserting new values that force rotations
  tree.insert(60);
  tree.insert(140);
  tree.insert(10);

  // Yanking the root
  tree.remove(100);

  // Verify the surviving nodes are still perfectly sorted and accessible
  EXPECT_EQ(tree.extractMinimum(), 10);
  EXPECT_EQ(tree.extractMinimum(), 50);
  EXPECT_EQ(tree.extractMinimum(), 60);
  EXPECT_EQ(tree.extractMinimum(), 125);
  EXPECT_EQ(tree.extractMinimum(), 140);
  EXPECT_EQ(tree.extractMinimum(), 175);
  EXPECT_THROW(tree.extractMinimum(), std::runtime_error);
}
