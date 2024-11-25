/**
 * \file math/interval.hpp
 **/
#ifndef OTHER_ENGINE_INTERVAL_HPP
#define OTHER_ENGINE_INTERVAL_HPP

#include <spdlog/fmt/fmt.h>

#include "core/formatters.hpp"
#include "math/vecmath.hpp"

namespace other {

  struct Interval {
    float min;
    float max;

    Interval()
        : min(-infinity<float>()), max(infinity<float>()) {}

    Interval(float min, float max)
        : min(min), max(max) {}

    Interval(const Interval& a, const Interval& b);

    double Size() const;
    double Clamp(double x) const;

    bool ClosedContains(double x) const;
    bool OpenContains(double x) const;

    Interval Expand(double delta) const;

    static const Interval empty;
    static const Interval universe;
  };

  Interval operator+(const Interval& ival, double displacement);
  Interval operator+(double displacement, const Interval& ival);

}  // namespace other

template <>
struct fmt::formatter<other::Interval> : public fmt::formatter<std::string_view> {
  auto format(const other::Interval& ival, fmt::format_context& ctx) {
    std::string res = other::fmtstr("Interval(min = {}, max = {})", ival.min, ival.max);
    return fmt::formatter<std::string_view>::format(res, ctx);
  }
};

#endif  // !OTHER_ENGINE_INTERVAL_HPP
