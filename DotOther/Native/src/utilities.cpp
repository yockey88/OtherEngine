/**
 * \file utilities.cpp
 **/
#include "utilities.hpp"

namespace dotother {
namespace util {
namespace detail {

  void UtilityObjects::SetLogLevel(MessageLevel level) {
    log_level = level;
  }

  void UtilityObjects::OverRideLogSink(internal_logging_hook_t sink) {
    log_sink = sink;
  }

  void UtilityObjects::ResetLogSink() {
    log_sink = nullptr;
  }

  bool UtilityObjects::IsVerbose() const {
    return log_level == MessageLevel::TRACE || 
           log_level == MessageLevel::DEBUG;
  }

} // namespace detail
} // namespace util
} // namespace dotother