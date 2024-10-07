/**
 * \file math/vecmath.hpp
 **/
#ifndef OTHER_ENGINE_VECMATH_HPP
#define OTHER_ENGINE_VECMATH_HPP

#include <glm/geometric.hpp>
#include <string_view>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/detail/qualifier.hpp>

#include <spdlog/fmt/fmt.h>

#include "core/defines.hpp"

template <typename T>
concept glm_t = std::is_same_v<glm::mat4 , T> || 
                std::is_same_v<glm::vec4 , T> || 
                std::is_same_v<glm::vec3 , T> || 
                std::is_same_v<glm::vec2 , T>;

namespace other {

  using Point2 = glm::vec2;
  using Point = glm::vec3;

} // namespace other
  
template <glm::length_t N , typename T, glm::qualifier Q>
struct fmt::formatter<glm::vec<N , T , Q>> : public fmt::formatter<std::string_view> {
  auto format(const glm::vec<N , T , Q>& vec , fmt::format_context& ctx) {
    std::stringstream ss;
    ss << "<";
    ss << vec.x;
    if constexpr (N >= 2) {
      ss << "," << vec.y;
    }

    if constexpr (N >= 3) {
      ss << "," << vec.z;
    }

    if constexpr (N >= 4) {
      ss << "," << vec.w;
    }
    ss << ">";
    return fmt::formatter<std::string_view>::format(other::fmtstr("{}" , ss.str()) , ctx);
  }
};

template <typename T>
  requires requires(T t) {
    T{0};
    std::numeric_limits<T>::epsilon();
  }
T EpsilonClamp(T val) {
  if (val < std::numeric_limits<T>::epsilon()) {
    return T{0};
  }
  return val;
}

template <glm::length_t N , typename T = float , glm::qualifier Q = glm::defaultp>
struct checked_difference {
  auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
    glm::vec<N , T , Q> diff = lhs - rhs;
    for (size_t i = 0; i < N; ++i) {
      diff[i] = EpsilonClamp(diff[i]);
    }
    return diff;
  }
};

template <glm::length_t N>
struct checked_difference<N , float , glm::defaultp> {
  auto operator()(const glm::vec<N , float , glm::defaultp>& lhs , const glm::vec<N , float , glm::defaultp>& rhs) const {
    glm::vec<N , float , glm::defaultp> diff = lhs - rhs;
    for (size_t i = 0; i < N; ++i) {
      diff[i] = EpsilonClamp<float>(diff[i]);
    }
    return diff;
  }
};

template <glm::length_t N , typename T , glm::qualifier Q>
struct checked_sum {
  auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
    glm::vec<N , T , Q> sum = lhs + rhs;
    for (size_t i = 0; i < N; ++i) {
      sum[i] = EpsilonClamp(sum[i]);
    }
    return sum;
  }
};

template <glm::length_t N>
struct checked_sum<N , float , glm::defaultp> {
  auto operator()(const glm::vec<N , float , glm::defaultp>& lhs , const glm::vec<N , float , glm::defaultp>& rhs) const {
    glm::vec<N , float , glm::defaultp> sum = lhs + rhs;
    for (size_t i = 0; i < N; ++i) {
      sum[i] = EpsilonClamp<float>(sum[i]);
    }
    return sum;
  }
};

template <glm::length_t N , typename T = float , glm::qualifier Q = glm::defaultp>
struct checked_normalize {
  auto operator()(const glm::vec<N , T , Q>& vec) const {
    for (size_t i = 0; i < N; ++i) {
      vec[i] = EpsilonClamp<T>(glm::normalize(vec[i]));
    }
    return vec;
  }
};


template <glm::length_t N>
struct checked_normalize<N , float , glm::defaultp> {
  auto operator()(const glm::vec<N , float , glm::defaultp>& vec) const {
    glm::vec<N , float , glm::defaultp> v = glm::normalize(vec);
    for (size_t i = 0; i < N; ++i) {
      v[i] = EpsilonClamp<float>(v[i]);
    }
    return v;
  }
};

template <glm::length_t N , typename T , glm::qualifier Q>
constexpr inline checked_difference<N , T  , Q> tvec_diff;
template <glm::length_t N>
constexpr inline checked_difference<N , float , glm::defaultp> nvec_diff;

template <glm::length_t N , typename T , glm::qualifier Q>
constexpr inline checked_sum<N , T , Q> tvec_sum;
template <glm::length_t N>
constexpr inline checked_sum<N , float , glm::defaultp> nvec_sum;

template <glm::length_t N , typename T , glm::qualifier Q>
constexpr inline checked_normalize<N , T , Q> tvec_norm;
template <glm::length_t N>
constexpr inline checked_normalize<N , float , glm::defaultp> nvec_norm;

  
template <>
struct fmt::formatter<glm::mat4> : public fmt::formatter<std::string_view> {
  auto format(const glm::mat4& mat , fmt::format_context& ctx) {
  constexpr std::string_view mat_str = 
R"(|{} {} {} {}|
|{} {} {} {}|
|{} {} {} {}|
|{} {} {} {}|)";

    std::string mat_fmt_str = fmt::format(
      fmt::runtime(mat_str) , 
      mat[0][0] , mat[1][0] , mat[2][0] , mat[3][0] ,
      mat[0][1] , mat[1][1] , mat[2][1] , mat[3][1] , 
      mat[0][2] , mat[1][2] , mat[2][2] , mat[3][2] ,
      mat[0][3] , mat[1][3] , mat[2][3] , mat[3][3]);
    return fmt::formatter<std::string_view>::format(mat_fmt_str , ctx); 
  }
};

#endif // !OTHER_ENGINE_VECMATH_HPP
