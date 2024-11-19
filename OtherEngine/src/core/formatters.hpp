/**
 * \file core/formatters.hpp
 **/
#ifndef OTHER_ENGINE_FORMATTERS_HPP
#define OTHER_ENGINE_FORMATTERS_HPP

#include <iostream>
#include <optional>
#include <string>
#include <string_view>

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
    return fmt::formatter<std::string_view>::format(fmt::format(std::string_view{ "{}" }, magic_enum::enum_name(e)), ctx);
  }
};

#endif  // !OTHER_ENGINE_FORMATTERS_HPP