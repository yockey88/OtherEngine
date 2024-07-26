/**
 * \file editor/editor_console_sink.cpp
 **/
#include "editor/editor_console_sink.hpp"

#include "core/defines.hpp"

#include "editor/console_panel.hpp"

namespace other {

  void EditorConsoleSink::sink_it_(const spdlog::details::log_msg& msg) {
    spdlog::memory_buf_t fmt_buf;
    spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg , fmt_buf);

    std::string message;
    for (auto& c : fmt_buf) {
      message.push_back(c);
    }

    std::string display_msg = message;

    if (display_msg.length() > 100) {
      size_t space_idx = display_msg.find_first_of(' ' , 100);
      if (space_idx != std::string::npos) {
        display_msg.replace(space_idx , display_msg.length() - 1, "...");
      }
    }

    message_buffer[size] = {
      .message = message , 
      .display_msg = display_msg ,
      .filters = GetFilters(msg.level) ,
      .time = std::chrono::system_clock::to_time_t(msg.time) ,
    };

    size++;

    if (size == capacity) {
      flush_();
    }
  }

  void EditorConsoleSink::flush_() {
    for (auto& m : message_buffer) {
      ConsolePanel::PushMessage(m);
    }

    size = 0;
  }
      
  uint8_t EditorConsoleSink::GetFilters(const spdlog::level::level_enum level) const {
    uint8_t filter = NO_FILTERS;
    switch (level) {
      case spdlog::level::level_enum::trace:
        [[fallthrough]];
      case spdlog::level::level_enum::debug:
        filter |= ValOf(DEBUG_FILTER);
        break;
      case spdlog::level::level_enum::info:
        filter |= ValOf(INFO_FILTER);
        break;
      case spdlog::level::level_enum::warn:
        filter |= ValOf(WARNING_FILTER);
        break;
      case spdlog::level::level_enum::err:
      case spdlog::level::level_enum::critical:
        filter |= ValOf(ERR_FILTER);
        break;
      default:
        break;
    }

    return filter;
  }

} // namespace other
