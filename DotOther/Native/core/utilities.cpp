/**
 * \file core/utilities.cpp
 **/
#include "core/utilities.hpp"

#include <print>

#include "core/hook_definitions.hpp"

namespace dotother {
namespace util {
  
  detail::UtilityObjects utils = {};

namespace detail {

  using namespace std::string_view_literals;
  void UtilityObjects::OverRideLogSink(native_log_callback_t sink) {
    log_sink = sink;
  }

  void UtilityObjects::ResetLogSink() {
    std::print("reseting log sink\n"sv);
    log_sink = nullptr;
  }

} // namespace detail
} // namespace util
} // namespace dotother