/**
 * \file math/vecmath.hpp
 **/
#ifndef OTHER_ENGINE_VECMATH_HPP
#define OTHER_ENGINE_VECMATH_HPP

#include <string_view>

#include <glm/glm.hpp>

#include <spdlog/fmt/fmt.h>

template <typename T>
concept glm_t = std::is_same_v<glm::mat4 , T> || 
                std::is_same_v<glm::vec4 , T> || 
                std::is_same_v<glm::vec3 , T> || 
                std::is_same_v<glm::vec2 , T>;

namespace other {

  using Point2 = glm::vec2;
  using Point = glm::vec3;

} // namespace other
  
template <>
struct fmt::formatter<glm::mat4> : public fmt::formatter<std::string_view> {
  auto format(const glm::mat4& mat , fmt::format_context& ctx) {
  constexpr std::string_view mat_str = 
R"(
[{} {} {} {}]
[{} {} {} {}]
[{} {} {} {}]
[{} {} {} {}]
)";

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
