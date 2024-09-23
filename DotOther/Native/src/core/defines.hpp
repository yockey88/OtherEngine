/**
 * \file core/defines.hpp 
 **/
#ifndef DOTOTHER_NATIVE_DEFINES_HPP
#define DOTOTHER_NATIVE_DEFINES_HPP

#include <memory>
#include <string_view>
#include <optional>

#include <spdlog/fmt/fmt.h>
#include <magic_enum.hpp>

#ifdef DOTOTHER_WINDOWS
  #include <Windows.h>
  #include <ShlObj_core.h>
  #define DOTOTHER_CALLTYPE __cdecl

  #ifdef _WCHAR_T_DEFINED
    #define DOTOTHER_HOSTFXR_NAME "hostfxr.dll"
    #define DO_STR(s) L##s
    #define DOTOTHER_WIDE_CHARS
  #else 
    #define DOTOTHER_HOSTFXR_NAME "hostfxr.dll"
    #define DO_STR(s) s
  #endif // _WCHAR_T_DEFINED
#else
  #define DOTOTHER_CALLTYPE
  #define DODOTOTHER_STR(s) s
  #define DO_HOSTFXR_NAME "libhostfxr.so"
#endif

#define DOTOTHER_DOTNET_TARGET_VERSION_MAJOR 8
#define DOTOTHER_DOTNET_TARGET_VERSION_MAJOR_STR '8'
#define DOTNET_UNMANAGED_FUNCTION UNMANAGEDCALLERSONLY_METHOD

namespace dotother {

#ifdef DOTOTHER_WIDE_CHARS
  using dochar = wchar_t;
#else
  using dochar = char;
#endif
  using dostring = std::basic_string<dochar>;
  using dostring_view = std::basic_string_view<dochar>;
  
  using dotypeid = int32_t;
  using dohandle = int32_t;
  using dobool = uint32_t;
  
  using nbool32 = uint32_t;
  
  template <typename T>
  using owner = std::unique_ptr<T>;
  
  template <typename T>
  using ref = std::shared_ptr<T>;
  
  template <typename T, typename... Args>
  owner<T> new_owner(Args&&... args) {
  	return std::make_unique<T>(std::forward<Args>(args)...);
  }
  
  template <typename T, typename... Args>
  ref<T> new_ref(Args&&... args) {
  	return std::make_shared<T>(std::forward<Args>(args)...);
  }
  
  template <typename T>
  using opt = std::optional<T>;
	
namespace literals {

  constexpr dostring_view operator""_sv(const wchar_t* _Str, size_t _Len) noexcept {
    return dostring_view(_Str, _Len);
  }

} // namespace literals

  enum class 
  MessageLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARNING = 3,
    ERR = 4,
    CRITICAL = 5,
    
    MESSAGE = 6,
  };
  
  enum class ManagedType {
    UNKNOWN,
    
    SBYTE,
    BYTE,
    SHORT,
    USHORT,
    INT,
    UINT,
    LONG,
    ULONG,
    
    FLOAT,
    DOUBLE,
    
    BOOL,
    
    POINTER,
  };
  
  enum class AssemblyLoadStatus {
    SUCCESS, 
    FILE_NOT_FOUND,
    FILE_LOAD_FAILED,
    INVALID_FILE_PATH,
    INVALID_ASSEMBLY,
    UNKNOWN_ERROR,
  };

  template<typename T>
  constexpr auto ValOf(T InValue) {
    return static_cast<std::underlying_type_t<T>>(InValue);
  }
  
  constexpr MessageLevel operator|(const MessageLevel lhs, const MessageLevel rhs) noexcept {
    return static_cast<MessageLevel>(ValOf(lhs) | ValOf(rhs));
  }
  
  constexpr bool operator&(const MessageLevel lhs, const MessageLevel rhs) noexcept {
    return (ValOf(lhs) & ValOf(rhs)) != 0;
  }
  
  constexpr MessageLevel operator~(const MessageLevel InValue) noexcept {
    return static_cast<MessageLevel>(~ValOf(InValue));
  }
  
  constexpr MessageLevel& operator|=(MessageLevel& lhs, const MessageLevel& rhs) noexcept {
    return (lhs = (lhs | rhs));
  }
  
  enum class TypeAccessibility {
    PUBLIC,
    PRIVATE,
    PROTECTED,
    INTERNAL,
    PROTECTED_PUBLIC,
    PRIVATE_PROTECTED
  };

} // namespace dotother

template <>
struct fmt::formatter<dotother::MessageLevel> : public fmt::formatter<std::string_view> {
  public:
    template <typename FormatContext>
    auto format(dotother::MessageLevel level, FormatContext& ctx) {
      std::string_view name = magic_enum::enum_name(level);
      return formatter<std::string_view>::format(name, ctx);
    }
};

#endif // !DOTOTHER_NATIVE_DEFINES_HPP
