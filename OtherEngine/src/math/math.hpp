/**
 * \file math/math.hpp
 **/
#ifndef OTHER_ENGINE_MATH_HPP
#define OTHER_ENGINE_MATH_HPP

#include <limits>

#include "core/logger.hpp"

namespace other {

  template <typename T>
  T Clamp(T val, double min, double max) {
    if (val < min) {
      return min;
    } else if (val > max) {
      return max;
    } else {
      return val;
    }
  }

  constexpr static inline size_t FastPow(size_t base, size_t exp) {
    size_t result = 1;
    while (exp > 0) {
      if (exp & 1) {
        result *= base;
      }
      base *= base;
      exp >>= 1;
    }
    return result;
  }

}  // namespace other

#endif  // !OTHER_ENGINE_MATH_HPP
