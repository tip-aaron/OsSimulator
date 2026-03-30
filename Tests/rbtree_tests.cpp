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