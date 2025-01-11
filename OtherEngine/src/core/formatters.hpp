/**
 * \file core/formatters.hpp
 **/
#ifndef OTHER_ENGINE_FORMATTERS_HPP
#define OTHER_ENGINE_FORMATTERS_HPP

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/fmt/fmt.h>

namespace other {

  template <typename... Args>
  static inline void println(const std::string_view format, Args&&... args) {
    std::cout << fmt::format(fmt::runtime(format), std::forward<Args>(args)...) << std::endl;
  }

  template <>
  inline void println(const std::string_view line) {
    std::cout << line << std::endl;
  }

  template <typename... Args>
  static inline auto fmtstr(const std::string_view format, Args&&... args) {
    return fmt::format(fmt::runtime(format), std::forward<Args>(args)...);
  }

  template <>
  inline auto fmtstr(const std::string_view line) {
    return std::string{ line };
  }

  template <typename... Args>
  static inline auto fmterr(const std::string_view format, Args&&... args) {
    /// TODO: something else...
    return fmtstr(format, std::forward<Args>(args)...);
  }

  template <typename T>
  static inline auto fmtopt(const std::string_view format, const std::optional<T>& opt) {
    if (opt.has_value()) {
      return fmtstr(format, opt.value());
    } else {
      return fmtstr("ERR");
    }
  }

}  // namespace other

template <typename E>
  requires std::is_enum_v<E>
struct fmt::formatter<E> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const E& e, FormatContext& ctx) {
    using namespace std::string_view_literals;
    using base_t = fmt::formatter<std::string_view>;
    std::string_view name = magic_enum::enum_name(e);
    if (name == "") {
      return base_t::format("UNKNOWN enum"sv, ctx);
    }
    return base_t::format(other::fmtstr("{}", name), ctx);
  }
};

template <>
struct fmt::formatter<glm::vec2> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const glm::vec2& v, FormatContext& ctx) {
    return fmt::formatter<std::string_view>::format(
      fmt::format(std::string_view{ "({:.2f}, {:.2f})" }, v.x, v.y), ctx
    );
  }
};

template <>
struct fmt::formatter<glm::vec3> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const glm::vec3& v, FormatContext& ctx) {
    return fmt::formatter<std::string_view>::format(
      fmt::format(std::string_view{ "({:.2f}, {:.2f}, {:.2f})" }, v.x, v.y, v.z), ctx
    );
  }
};

template <>
struct fmt::formatter<glm::vec4> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const glm::vec4& v, FormatContext& ctx) {
    return fmt::formatter<std::string_view>::format(
      fmt::format(std::string_view{ "({:.2f}, {:.2f}, {:.2f}, {:.2f})" }, v.x, v.y, v.z, v.w), ctx
    );
  }
};

template <>
struct fmt::formatter<glm::quat> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const glm::quat& q, FormatContext& ctx) {
    return fmt::formatter<std::string_view>::format(
      fmt::format(std::string_view{ "({:.2f}, {:.2f}, {:.2f}, {:.2f})" }, q.x, q.y, q.z, q.w), ctx
    );
  }
};

/// TODO: make this better, sometimes columns are not aligned
template <>
struct fmt::formatter<glm::mat4> : public fmt::formatter<std::string_view> {
  auto format(const glm::mat4& mat, fmt::format_context& ctx) {
    constexpr std::string_view mat_str = "|{} {} {} {}|\n|{} {} {} {}|\n|{} {} {} {}|\n|{} {} {} {}|";

    std::string mat_fmt_str = fmt::format(
      fmt::runtime(mat_str), mat[0][0], mat[1][0], mat[2][0], mat[3][0],
      mat[0][1], mat[1][1], mat[2][1], mat[3][1], mat[0][2], mat[1][2],
      mat[2][2], mat[3][2], mat[0][3], mat[1][3], mat[2][3], mat[3][3]
    );
    return fmt::formatter<std::string_view>::format(mat_fmt_str, ctx);
  }
};

#endif  // !OTHER_ENGINE_FORMATTERS_HPP
