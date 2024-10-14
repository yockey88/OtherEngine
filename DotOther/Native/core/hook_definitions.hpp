/**
 * \file core/hook_definitions.hpp
 **/
#ifndef DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP
#define DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP

#include <string_view>

#include "core/dotother_defines.hpp"

#include "hosting/native_string.hpp"


namespace dotother {

  using exception_callback_t = void (*)(const NString message);
  using log_callback_t = void (*)(const NString message, MessageLevel level);
  using native_log_callback_t = void (*)(const std::string_view message, MessageLevel level);
  using invoke_native_method_hook_t = void (*)(uint64_t, const NString method_name);
  using coreclr_error_callback_t = void (*)(const dochar* message);
  using retrieve_native_object_hook_t = void* (*)(uint64_t);

}  // namespace dotother

#endif  // !DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP
