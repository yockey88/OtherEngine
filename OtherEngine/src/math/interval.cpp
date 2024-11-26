/**
 * \file math/interval.cpp
 **/
#include "math/interval.hpp"

namespace other {

  const Interval Interval::empty = Interval(+infinity<float>(), -infinity<float>());
  const Interval Interval::universe = Interval();

  Interval::Interval(const Interval& a, const Interval& b) {
    min = a.min <= b.min ?
      a.min :
      b.min;
    max = a.max >= b.max ?
      a.max :
      b.max;
  }

  double Interval::Size() const {
    return max - min;
  }

  double Interval::Clamp(double x) const {
    if (x < min) return min;
    if (x > max) return max;
    return x;
  }

  bool Interval::ClosedContains(double x) const {
    return min <= x && x <= max;
  }

  bool Interval::OpenContains(double x) const {
    return min < x && x < max;
  }

  Interval Interval::Expand(double delta) const {
    auto padding = delta / 2;
    return Interval(min - padding, max + padding);
  }

  Interval operator+(const Interval& ival, double displacement) {
    return Interval(ival.min + displacement, ival.max + displacement);
  }

  Interval operator+(double displacement, const Interval& ival) {
    return ival + displacement;
  }

}  // namespace other
