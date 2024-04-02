/**
 * \file octree-tests/octree_tests_helpers.hpp
 **/
#ifndef OTHER_ENGINE_OCTREE_TESTS_HELPERS_HPP
#define OTHER_ENGINE_OCTREE_TESTS_HELPERS_HPP

#include <cmath>

/// closed interval [0...n]
static size_t GetSeriesSum(size_t base , size_t range) {
  size_t sum = 0;
  for (size_t i = 0; i <= range; ++i) {
    sum += std::pow(base, i);
  }
  return sum;
}

#endif // !OTHER_ENGINE_OCTREE_TESTS_HELPERS_HPP
