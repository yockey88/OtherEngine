/**
 * \file core/hook_definitions.hpp
 **/
#ifndef DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP
#define DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP

#include <string_view>

#include "core/defines.hpp"
#include "hosting/native_string.hpp"

namespace dotother {

  using exception_callback_t = void(*)(const NString message);
  using log_callback_t = void(*)(const NString message, MessageLevel level);
  using invoke_native_method_hook_t = void(*)(uint64_t,const NString method_name);

  using coreclr_error_callback_t = void(*)(const dochar* message);
  using internal_logging_hook_t = std::function<void(const std::string_view message, MessageLevel level, bool verbose)>;
  

} // namespace dotother

#endif // !DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP
