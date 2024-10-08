/**
 * \file math/vecmath.hpp
 **/
#ifndef OTHER_ENGINE_VECMATH_HPP
#define OTHER_ENGINE_VECMATH_HPP

#include <glm/geometric.hpp>
#include <limits>
#include <string_view>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/detail/qualifier.hpp>

#include <spdlog/fmt/fmt.h>

#include "core/defines.hpp"

namespace other {

  template <typename T>
  concept glm_vec_t = std::is_same_v<glm::vec4 , T> || 
                      std::is_same_v<glm::vec3 , T> || 
                      std::is_same_v<glm::vec2 , T>;

  template <typename T>
  concept glm_mat_t = std::is_same_v<glm::mat4 , T> || 
                      std::is_same_v<glm::mat3 , T> || 
                      std::is_same_v<glm::mat2 , T>;
  
  template <typename T>
  concept glm_t = glm_vec_t<T> || glm_mat_t<T>;


  using Point2 = glm::vec2;
  using Point = glm::vec3;

  template <typename... Ts>
  concept zero_initializable = requires(Ts...) {
    (Ts{0} , ...);
  };

  template <typename T , typename U>
  concept subtractable = requires(T t , U u) {
    t - u;
  };

  template <typename T , typename U>
  concept addable = requires(T t , U u) {
    t + u;
  };

  template <typename T , typename U>
  concept product_defined = requires(T t , U u) {
    t * u;
  };

  template <typename T , typename U>
  concept measurable = 
    subtractable<T , U> && 
    requires(T t , U u) {
      { std::abs(t - u) } -> std::convertible_to<float>;
    };

  template <typename T , typename U>
  concept vec_measureable = 
    subtractable<T , U> && 
    requires (T t , U u) {
      { glm::length(t - u) } -> std::convertible_to<float>;
    };

  template <typename T , typename U>
  concept norm_defnd = vec_measureable<T , U>;

  template <typename T>
  concept epsilon_defined = requires(T t) {
    std::numeric_limits<T>::epsilon();
  };

  template <typename T>
  concept epsilon_comparable = 
    epsilon_defined<T> && zero_initializable<T> &&
    requires(T t) {
      t < std::numeric_limits<T>::epsilon();
    };

  template <typename T , typename U>
  concept comparable = requires(T t , U u) {
    t == u;
    t < u;
    t > u;
  };

  template <typename... Ts>
  concept multi_epsilon_comparable = (epsilon_comparable<Ts> && ...);

  template <typename T , typename U>
  concept epsilon_comparable_with = comparable<T , U> && multi_epsilon_comparable<T , U>;

  template <typename T>
    requires epsilon_defined<T>
  constexpr T eps() {
    return std::numeric_limits<T>::epsilon();
  }

  template <typename T>
    requires epsilon_comparable<T>
  constexpr bool EpsilonZero(T val) {
    return val <= eps<T>() && val >= -eps<T>();
  }

  template <typename T>
    requires epsilon_comparable<T>
  constexpr T EpsilonClamp(T val) {
    return EpsilonZero(val) ? T{0} : val;
  }

  template <typename T , typename U>
    requires subtractable<T , U> && epsilon_comparable_with<T , U>
  constexpr std::common_type_t<T , U> EpsilonSubtract(T lhs , U rhs) {
    using result_t = std::common_type_t<T , U>;
    return EpsilonClamp<result_t>(lhs - rhs);
  }

  template <typename T , typename U>
    requires addable<T , U> && epsilon_comparable_with<T , U>
  constexpr T EpsilonAdd(T lhs , U rhs) {
    using result_t = std::common_type_t<T , U>;
    return EpsilonClamp<result_t>(lhs + rhs);
  }

  template <typename T>
    requires epsilon_comparable<T>
  constexpr T EpsilonAbs(T val) {
    if (EpsilonZero(val)) {
      return T{0};
    }
    return val < T{0} ? -val : val;
  }

  template <typename T , typename U>
    requires measurable<T , U> && epsilon_comparable_with<T , U>
  constexpr T EpsilonDifference(T lhs , U rhs) {
    using result_t = std::common_type_t<T , U>;
    return EpsilonAbs<result_t>(EpsilonClamp<result_t>(lhs - rhs));
  }

  template <typename T , typename U>
    requires product_defined<T , U> && epsilon_comparable_with<T , U>
  constexpr T EpsilonProduct(T lhs , U rhs) {
    using result_t = std::common_type_t<T , U>;
    return EpsilonClamp<result_t>(lhs * rhs);
  }

  template <typename T , typename U>
    requires multi_epsilon_comparable<T , U>
  constexpr bool EpsilonEqual(T lhs , U rhs) {
    return EpsilonDifference<T , U>(lhs , rhs) == 0;
  }

  template <typename T , typename U>
    requires multi_epsilon_comparable<T , U>
  constexpr bool EpsilonLt(T lhs , U rhs) {
    return EpsilonSubtract<T , U>(lhs , rhs) < 0;
  }

  template <typename T , typename U>
    requires multi_epsilon_comparable<T , U>
  constexpr bool EpsilonGt(T lhs , U rhs) {
    return EpsilonSubtract(lhs , rhs) > 0;
  }
  
  template <glm::length_t N , typename T , glm::qualifier Q>
  struct lexicographical_order {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      for (size_t i = 0; i < N; ++i) {
        if (lhs[i] < rhs[i]) {
          return -1;
        } else if (lhs[i] > rhs[i]) {
          return 1;
        }
      }
      return 0;
    }
  };
  constexpr inline lexicographical_order<2 , float , glm::defaultp> vec2_order;
  constexpr inline lexicographical_order<3 , float , glm::defaultp> vec3_order;
  constexpr inline lexicographical_order<4 , float , glm::defaultp> vec4_order;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct vec_epsilon_lt {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      bool result = false;
      for (size_t i = 0; i < N; ++i) {
        result = EpsilonLt(lhs[i] , rhs[i]);
        if (!result) {
          break;
        }
      }
      return result;
    }
  };
  constexpr inline vec_epsilon_lt<2 , float , glm::defaultp> vec2_lt;
  constexpr inline vec_epsilon_lt<3 , float , glm::defaultp> vec3_lt;
  constexpr inline vec_epsilon_lt<4 , float , glm::defaultp> vec4_lt;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct vec_epsilon_gt {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      bool result = false;
      for (size_t i = 0; i < N; ++i) {
        result = EpsilonGt(lhs[i] , rhs[i]);
        if (!result) {
          break;
        }
      }
      return result;
    }
  };
  constexpr inline vec_epsilon_gt<2 , float , glm::defaultp> vec2_gt;
  constexpr inline vec_epsilon_gt<3 , float , glm::defaultp> vec3_gt;
  constexpr inline vec_epsilon_gt<4 , float , glm::defaultp> vec4_gt;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct vec_epsilon_lte {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      return !vec_epsilon_gt<N , T , Q>{}(lhs , rhs);
    }
  };
  constexpr inline vec_epsilon_lte<2 , float , glm::defaultp> vec2_lte;
  constexpr inline vec_epsilon_lte<3 , float , glm::defaultp> vec3_lte;
  constexpr inline vec_epsilon_lte<4 , float , glm::defaultp> vec4_lte;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct vec_epsilon_gte {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      return !vec_epsilon_lt<N , T , Q>{}(lhs , rhs);
    }
  };
  constexpr inline vec_epsilon_gte<2 , float , glm::defaultp> vec2_gte;
  constexpr inline vec_epsilon_gte<3 , float , glm::defaultp> vec3_gte;
  constexpr inline vec_epsilon_gte<4 , float , glm::defaultp> vec4_gte;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct vec_epsilon_eq {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      bool result = true;
      for (size_t i = 0; i < N; ++i) {
        result = EpsilonEqual(lhs[i] , rhs[i]);
        if (!result) {
          break;
        }
      }
      return result;
    }
  };
  constexpr inline vec_epsilon_eq<2 , float , glm::defaultp> vec2_eq;
  constexpr inline vec_epsilon_eq<3 , float , glm::defaultp> vec3_eq;
  constexpr inline vec_epsilon_eq<4 , float , glm::defaultp> vec4_eq;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct checked_min {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      return vec_epsilon_lt<N , T , Q>{}(lhs , rhs) ? lhs : rhs;
    }
  };
  constexpr inline checked_min<2 , float , glm::defaultp> vec2_min;
  constexpr inline checked_min<3 , float , glm::defaultp> vec3_min;
  constexpr inline checked_min<4 , float , glm::defaultp> vec4_min;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct checked_max {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      return vec_epsilon_gt<N , T , Q>{}(lhs , rhs) ? lhs : rhs;
    }
  };
  constexpr inline checked_max<2 , float , glm::defaultp> vec2_max;
  constexpr inline checked_max<3 , float , glm::defaultp> vec3_max;
  constexpr inline checked_max<4 , float , glm::defaultp> vec4_max;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct checked_subtract {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      glm::vec<N , T , Q> diff;
      for (size_t i = 0; i < N; ++i) {
        diff[i] = EpsilonSubtract(lhs[i], rhs[i]);
      }
      return diff;
    }

    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , T rhs) const {
      glm::vec<N , T , Q> diff;
      for (size_t i = 0; i < N; ++i) {
        diff[i] = EpsilonSubtract(lhs[i] , rhs);
      }
      return diff;
    }

    constexpr auto operator()(T lhs , const glm::vec<N , T , Q>& rhs) const {
      glm::vec<N , T , Q> diff;
      for (size_t i = 0; i < N; ++i) {
        diff[i] = EpsilonSubtract(lhs , rhs[i]);
      }
      return diff;
    }
  };
  constexpr inline checked_subtract<2 , float , glm::defaultp> vec2_sub;
  constexpr inline checked_subtract<3 , float , glm::defaultp> vec3_sub;
  constexpr inline checked_subtract<4 , float , glm::defaultp> vec4_sub;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct checked_sum {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      glm::vec<N , T , Q> sum;
      for (size_t i = 0; i < N; ++i) {
        sum[i] = EpsilonAdd(lhs[i] , rhs[i]);
      }
      return sum;
    }

    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , T rhs) const {
      glm::vec<N , T , Q> sum;
      for (size_t i = 0; i < N; ++i) {
        sum[i] = EpsilonAdd(lhs[i] , rhs);
      }
      return sum;
    }

    constexpr auto operator()(T lhs , const glm::vec<N , T , Q>& rhs) const {
      glm::vec<N , T , Q> sum;
      for (size_t i = 0; i < N; ++i) {
        sum[i] = EpsilonAdd(lhs , rhs[i]);
      }
      return sum;
    }
  };
  constexpr inline checked_sum<2 , float , glm::defaultp> vec2_sum;
  constexpr inline checked_sum<3 , float , glm::defaultp> vec3_sum;
  constexpr inline checked_sum<4 , float , glm::defaultp> vec4_sum;

  template <glm::length_t N , typename T , glm::qualifier Q>
  struct checked_difference {
    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , const glm::vec<N , T , Q>& rhs) const {
      glm::vec<N , T , Q> diff;
      for (size_t i = 0; i < N; ++i) {
        diff[i] = EpsilonDifference(lhs[i] , rhs[i]);
      }
      return diff;
    }
  };
  constexpr inline checked_difference<2 , float , glm::defaultp> vec2_diff;
  constexpr inline checked_difference<3 , float , glm::defaultp> vec3_diff;
  constexpr inline checked_difference<4 , float , glm::defaultp> vec4_diff;
  
  template <glm::length_t N , typename T , glm::qualifier Q>
  struct checked_product {
    constexpr auto operator()(float lhs , const glm::vec<N , T , Q>& rhs) const {
      glm::vec<N , T , Q> product;
      for (size_t i = 0; i < N; ++i) {
        product[i] = EpsilonProduct(lhs , rhs[i]);
      }
      return product;
    }

    constexpr auto operator()(const glm::vec<N , T , Q>& lhs , float rhs) const {
      glm::vec<N , T , Q> product;
      for (size_t i = 0; i < N; ++i) {
        product[i] = EpsilonProduct(lhs[i] , rhs);
      }
      return product;
    }
  };
  constexpr inline checked_product<2 , float , glm::defaultp> vec2_product;
  constexpr inline checked_product<3 , float , glm::defaultp> vec3_product;
  constexpr inline checked_product<4 , float , glm::defaultp> vec4_product;
  
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
