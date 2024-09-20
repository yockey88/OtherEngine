/**
 * \file hook_definitions.hpp
 **/
#ifndef DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP
#define DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP

#include <string_view>

#include "defines.hpp"
#include "native_string.hpp"

namespace dotother {

	using exception_callback_t = void(*)(const NString message);
	using log_callback_t = void(*)(const NString message, MessageLevel level);

  using coreclr_error_callback_t = void(*)(const dochar* message);
  using internal_logging_hook_t = void(*)(const std::string_view message, MessageLevel level, bool verbose);

} // namespace dotother

#endif // !DOTOTHER_NATIVE_HOOK_DEFINITIONS_HPP