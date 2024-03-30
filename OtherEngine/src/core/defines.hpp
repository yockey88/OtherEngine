/**
 * \file core/defines.hpp
*/
#ifndef OTHER_ENGINE_DEFINES_HPP
#define OTHER_ENGINE_DEFINES_HPP

#include <string_view>
#include <cstdint>
#include <memory>
#include <optional>
#include <iostream>

#include <spdlog/fmt/fmt.h>
#include <glm/glm.hpp>

#define bit(x) (1 << x)

#ifdef OE_MODULE
  #define OE_CLIENT
#else
  #define OE_ENGINE
#endif

namespace other {
  
  enum ExitCode : uint8_t {
    SUCCESS = 0x00 ,
    RELOAD_PROJECT ,
    LOAD_NEW_PROJECT ,

    NUM_EXIT_CODES ,
    FAILURE = NUM_EXIT_CODES ,
    INVALID = FAILURE
  };

  template <typename T>
  using Scope = std::unique_ptr<T>;
  
  template <typename T>
  using StdRef = std::shared_ptr<T>;

  template <typename T>
  using Opt = std::optional<T>;

  template <typename T , typename... Args>
  Scope<T> NewScope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }

  template <typename T , typename... Args>
  StdRef<T> NewStdRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }

  template <typename T>
  struct Result {
    Opt<T> value;
    Opt<std::string> error;

    inline bool IsOk() const { return value.has_value(); }
    inline bool IsErr() const { return error.has_value(); }

    inline T& Unwrap() { return value.value(); }
    inline const T& Unwrap() const { return value.value(); }
    
    inline std::string& Error() { return error.value(); }
  };

  static constexpr uint64_t kFnvOffsetBasis = 0xBCF29CE484222325;
  static constexpr uint64_t kFnvPrime = 0x100000001B3;
  
  static constexpr uint64_t FNV(std::string_view str) {
    uint64_t hash = kFnvOffsetBasis;
    for (auto& c : str) {
      hash ^= c;
      hash *= kFnvPrime;
    }
    hash ^= str.length();
    hash *= kFnvPrime;
  
    return hash;
  }

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
  static inline auto fmtopt(const std::string_view format, const Opt<T>& opt) {
    if (opt.has_value()) {
      return fmtstr(format, opt.value());
    } else {
      return fmtstr("ERR"); 
    }
  }

} // namespace other

template <>
struct fmt::formatter<glm::vec4> : public fmt::formatter<std::string_view> {
  template <typename FormatContext>
  auto format(const glm::vec4& v, FormatContext& ctx) {
    return fmt::formatter<std::string_view>::format(
        fmt::format(std::string_view{ "({:.2f}, {:.2f}, {:.2f}, {:.2f})" } , v.x , v.y , v.z , v.w), ctx);
  }
};

#endif // !OTHER_ENGINE_DEFINES_HPP
