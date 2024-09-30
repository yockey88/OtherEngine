/**
 * \file core/utilities.cpp
 **/
#include "core/utilities.hpp"

#include <print>

namespace dotother {
namespace util {
namespace detail {

  void UtilityObjects::SetLogLevel(MessageLevel level) {
    log_level = level;
  }

  using namespace std::string_view_literals;
  void UtilityObjects::OverRideLogSink(internal_logging_hook_t sink) {
    std::print("Overriding log sink\n"sv);
    log_sink = sink;
  }

  void UtilityObjects::ResetLogSink() {
    std::print("reseting log sink\n"sv);
    log_sink = nullptr;
  }

  bool UtilityObjects::IsVerbose() const {
    return log_level == MessageLevel::TRACE || 
           log_level == MessageLevel::DEBUG;
  }

} // namespace detail
} // namespace util
} // namespace dotother