/**
 * \file math/math.hpp
 **/
#ifndef OTHER_ENGINE_MATH_HPP
#define OTHER_ENGINE_MATH_HPP

#include <limits>

#include "core/logger.hpp"

namespace other {

  template <typename T>
  T Clamp(T val , double min , double max) {
    if (val < min) {
      return min;
    } else if (val > max) {
      return max;
    } else {
      return val;
    }
  }

} // namespace other

#endif // !OTHER_ENGINE_MATH_HPP
