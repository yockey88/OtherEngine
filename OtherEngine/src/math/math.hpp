/**
 * \file math/math.hpp
 **/
#ifndef OTHER_ENGINE_MATH_HPP
#define OTHER_ENGINE_MATH_HPP

namespace other {

  // Definitions of useful mathematical constants
  //
  // Define _USE_MATH_DEFINES before including <math.h> to expose these macro
  // definitions for common math constants.  These are placed under an #ifdef
  // since these commonly-defined names are not part of the C or C++ standards

  namespace constants {

    constexpr static double e = 2.71828182845904523536;        // e
    constexpr static double log2e = 1.44269504088896340736;    // log2(e)
    constexpr static double log10e = 0.434294481903251827651;  // log10(e)
    constexpr static double ln2 = 0.693147180559945309417;     // ln(2)
    constexpr static double ln10 = 2.30258509299404568402;     // ln(10)
    constexpr static double pi = 3.14159265358979323846;       // pi
    constexpr static double pi2 = 1.57079632679489661923;      // pi/2
    constexpr static double pi4 = 0.785398163397448309616;     // pi/4
                                                               // constexpr static double M_1_PI 0.318309886183790671538     // 1/pi
                                                               // constexpr static double M_2_PI 0.636619772367581343076     // 2/pi
                                                               // constexpr static double M_2_SQRTPI 1.12837916709551257390  // 2/sqrt(pi)
                                                               // constexpr static double M_SQRT2 1.41421356237309504880     // sqrt(2)
                                                               // constexpr static double M_SQRT1_2 0.707106781186547524401  // 1/sqrt(2)

  }  // namespace constants

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
