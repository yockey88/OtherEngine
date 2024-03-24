/**
 * \file core/ref_counted.cpp
 */
#include "core/ref_counted.hpp"

namespace other {

  void RefCounted::Increment() {
    count++;
  }

  void RefCounted::Decrement() {
    count--;
  }

} // namespace other
