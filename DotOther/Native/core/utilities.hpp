/**
 * \file core/utilities.hpp
 **/
#ifndef DOTOTHER_NATIVE_UTILITIES_HPP
#define DOTOTHER_NATIVE_UTILITIES_HPP

#include <iostream>
#include <string_view>

#include "core/dotother_defines.hpp"
#include "core/hook_definitions.hpp"

namespace dotother {
namespace util { 
namespace detail {
	
  struct UtilityObjects {
    MessageLevel log_level = MessageLevel::MESSAGE;

    internal_logging_hook_t log_sink = nullptr;

    void SetLogLevel(MessageLevel level);
    void OverRideLogSink(internal_logging_hook_t sink);
    void ResetLogSink();
    bool IsVerbose() const;
  };

} // namespace detail

  static inline detail::UtilityObjects& GetUtils() {
    static detail::UtilityObjects utils;
    return utils;
  };

#ifdef DOTOTHER_WIDE_CHARS
  std::wstring CharToWide(const std::string_view str);
  std::string WideToChar(const std::wstring_view str);
#else
  std::string CharToWide(const std::string_view str);
  std::string WideToChar(const std::wstring_view str);
#endif // DOTOTHER_WIDE_CHARS
  
  template <typename... Args>
  std::string format(dostring_view fmt , Args&&... args) {
    return fmt::format(fmt::runtime(
#ifdef DOTOTHER_WIDE_CHARS
                      WideToChar(fmt)
#else
                        fmt
#endif // DOTOTHER_WIDE_CHARS
                      ), std::forward<Args>(args)...);
  }

namespace {
  
  template <typename... Args>
  static void sink_message(MessageLevel level , dostring_view fmt, Args&&... args) {
    auto& utils = GetUtils();
    if (!utils.IsVerbose() && level < utils.log_level) {
      return;
    }

    if (utils.log_sink == nullptr) {
      std::cout << fmt::format(fmt::runtime(
#ifdef DOTOTHER_WIDE_CHARS
                      WideToChar(fmt))
#else
                        fmt
#endif
                        , std::forward<Args>(args)...) << "\n";
      return;
    }
    utils.log_sink(format(fmt, std::forward<Args>(args)...), MessageLevel::TRACE, utils.IsVerbose());
  }

} // anonymous namespace

  template <typename... Args>
  static void print(dostring_view fmt, MessageLevel level, Args&&... args) {
    sink_message(level, fmt, std::forward<Args>(args)...);
  }
  
  template <>
  void print(dostring_view fmt, MessageLevel level) {
    sink_message(level , fmt);
  }
  
  template<typename TArg>
  constexpr ManagedType GetManagedType() {
    if constexpr (std::is_pointer_v<std::remove_reference_t<TArg>>) {
      return ManagedType::POINTER;
    } else if constexpr (std::same_as<TArg, uint8_t> || std::same_as<TArg, std::byte>) {
      return ManagedType::BYTE;
    } else if constexpr (std::same_as<TArg, uint16_t>) {
      return ManagedType::USHORT;
    } else if constexpr (std::same_as<TArg, uint32_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 4)) {
      return ManagedType::UINT;
    } else if constexpr (std::same_as<TArg, uint64_t> || (std::same_as<TArg, unsigned long> && sizeof(TArg) == 8)) {
      return ManagedType::ULONG;
    } else if constexpr (std::same_as<TArg, char8_t>) {
      return ManagedType::SBYTE;
    } else if constexpr (std::same_as<TArg, int16_t>) {
      return ManagedType::SHORT;
    } else if constexpr (std::same_as<TArg, int32_t> || (std::same_as<TArg, long> && sizeof(TArg) == 4)) {
      return ManagedType::INT;
    } else if constexpr (std::same_as<TArg, int64_t> || (std::same_as<TArg, long> && sizeof(TArg) == 8)){
      return ManagedType::LONG;
    } else if constexpr (std::same_as<TArg, float>) {
      return ManagedType::FLOAT;
    } else if constexpr (std::same_as<TArg, double>) {
      return ManagedType::DOUBLE;
    } else if constexpr (std::same_as<TArg, bool>) {
      return ManagedType::BOOL;
    } else {
      return ManagedType::UNKNOWN;
    }
  }
  
  template <typename A, size_t I>
  inline void AddToArrayAt(const void** args_arr, ManagedType* param_types, A&& InArg) {
    param_types[I] = GetManagedType<A>();
    if constexpr (std::is_pointer_v<std::remove_reference_t<A>>) {
      args_arr[I] = reinterpret_cast<const void*>(InArg);
    } else {
      args_arr[I] = reinterpret_cast<const void*>(&InArg);
    }
  }
  
  template <typename... Args, size_t... Is>
  inline void AddToArray(const void** args, ManagedType* parameters, Args&&... values, const std::index_sequence<Is...>&) {
    (AddToArrayAt<Args, Is>(args, parameters, std::forward<Args>(values)), ...);
  }

} // namespace util
} // namespace dotother

#endif // !DOTOTHER_NATIVE_UTILITIES_HPP
