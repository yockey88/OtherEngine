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
#include <filesystem>
#include <utility>

#include <spdlog/fmt/fmt.h>
#include <glm/glm.hpp>
#include <type_traits>
#include <utility>

#define bit(x) (1 << x)

#ifdef OE_MODULE
  #define OE_CLIENT
#else
  #define OE_ENGINE
#endif

#define FMT_ENUM(e) \
  case other::e: \
    return fmt::formatter<std::string_view>::format("other::" #e , ctx);

namespace other {
  
  enum ExitCode : uint8_t {
    SUCCESS = 0x00 ,
    RELOAD_PROJECT ,
    LOAD_NEW_PROJECT ,

    NUM_EXIT_CODES ,
    FAILURE = NUM_EXIT_CODES ,
    INVALID = FAILURE
  };
  
  enum ValueType {
    EMPTY , // Void , null , nil ,etc...
            
    /// primitive types
    BOOL , CHAR ,
    INT8 , INT16 , INT32 , INT64 ,
    UINT8 , UINT16 , UINT32 , UINT64 ,
    FLOAT , DOUBLE ,
    STRING ,

    /// engine types
    VEC2 , VEC3 , VEC4 , 
    
    MAT2 , MAT3 , MAT4 ,

    SAMPLER2D ,

    ASSET , ENTITY ,

    /// user types
    USER_TYPE ,
  };
  
  template <typename T>
  static constexpr ValueType GetValueType() {
    if constexpr (std::is_same_v<T , bool>) {
      return ValueType::BOOL;
    } else if constexpr (std::is_same_v<T , char>) {
      return ValueType::CHAR;
    } else if constexpr (std::is_same_v<T , int8_t>) {
      return ValueType::INT8;
    } else if constexpr (std::is_same_v<T, int16_t>) {
      return ValueType::INT16;
    } else if constexpr (std::is_same_v<T, int32_t>) {
      return ValueType::INT32;
    } else if constexpr (std::is_same_v<T, int64_t>) {
      return ValueType::INT64;
    } else if constexpr (std::is_same_v<T, uint8_t>) {
      return ValueType::UINT8;
    } else if constexpr (std::is_same_v<T, uint16_t>) {
      return ValueType::UINT16;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      return ValueType::UINT32;
    } else if constexpr (std::is_same_v<T, uint64_t>) {
      return ValueType::UINT64;
    } else if constexpr (std::is_same_v<T, float>) {
      return ValueType::FLOAT;
    } else if constexpr (std::is_same_v<T, double>) {
      return ValueType::DOUBLE;
    } else if constexpr (std::is_same_v<T , glm::vec2>) {
      return ValueType::VEC2;
    } else if constexpr (std::is_same_v<T , glm::vec3>) {
      return ValueType::VEC3;
    } else if constexpr (std::is_same_v<T , glm::vec4>) {
      return ValueType::VEC4;
    } else if constexpr (std::is_same_v<T , glm::mat2>) {
      return ValueType::MAT2;
    } else if constexpr (std::is_same_v<T , glm::mat3>) {
      return ValueType::MAT3;
    } else if constexpr (std::is_same_v<T , glm::mat4>) {
      return ValueType::MAT4;
    } else {
      return ValueType::EMPTY;
    }
  }

  static constexpr size_t GetValueSize(ValueType type) {
    switch (type) {
      case BOOL: 
      case CHAR: 
      case INT8: 
      case UINT8: 
        return 1;
      
      case INT16: 
      case UINT16: 
        return 2;
      
      case INT32: 
      case UINT32: 
      case FLOAT:
      case SAMPLER2D:
        return 4;
      
      case INT64: 
      case UINT64: 
      case DOUBLE:
        /// these are here because they are UUIDs which are uint64
      case ASSET:
      case ENTITY:
        return 8;

      case VEC2:
        return 2 * 4;
      case VEC3:
        return 3 * 4;
      case VEC4:
        return 4 * 4;
      
      case MAT2:
        return 2 * 2 * 4;
      case MAT3:
        return 3 * 3 * 4;
      case MAT4:
        return 4 * 4 * 4;

      default:
        return 0;
    }
  }

  template <typename T>
  using Scope = std::unique_ptr<T>;
  
  template <typename T>
  using StdRef = std::shared_ptr<T>;

  template <typename T>
  using Opt = std::optional<T>;

  using Path = std::filesystem::path;

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

  template <typename T>
  concept is_enum = std::is_enum_v<T>;

  template <typename E>
    requires is_enum<E>
  constexpr std::underlying_type_t<E> ValOf(E e) {
    /// solve really annoying LSP issue
    return fmt::underlying(e);
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
