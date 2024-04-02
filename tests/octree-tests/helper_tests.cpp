/**
 * \file octree-tests/helper_tests.cpp
 **/
#include "octree_tests_helpers.hpp"

#include <gtest/gtest.h>

TEST(SeriesSumTests , series_sum_0) {
  size_t base = 8;
  size_t range = 0;
  size_t sum = GetSeriesSum(base , range);
  ASSERT_EQ(sum , 1);
}

TEST(SeriesSumTests , series_sum) {
  size_t base = 8;
  size_t range = 3;
  size_t sum = GetSeriesSum(base , range);
  ASSERT_EQ(sum , 1 + 8 + 64 + 512);
}

TEST(SeriesSumTests , series_sum_1) {
  size_t base = 8;
  size_t range = 1;
  size_t sum = GetSeriesSum(base , range);
  ASSERT_EQ(sum , 1 + 8);
}

TEST(SeriesSumTests , series_sum_5) {
  size_t base = 8;
  size_t range = 5;
  size_t sum = GetSeriesSum(base , range);
  ASSERT_EQ(sum , 1 + 8 + 64 + 512 + 4096 + 32768);
}

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
