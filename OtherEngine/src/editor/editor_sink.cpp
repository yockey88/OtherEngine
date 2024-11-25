/**
 * \file editor/editor_sink.cpp
 **/
#include "editor/editor_sink.hpp"

#include "core/defines.hpp"
#include "editor/panels/log_panel.hpp"

namespace other {

  std::string GetDisplayMsgFromMessage(const std::string& message) {
    if (message.length() > 100) {
      size_t space_idx = message.find_first_of(' ', 100);
      if (space_idx != std::string::npos) {
        return message.substr(0, space_idx) + "...";
      }
    }

    return message;
  }

  void EditorSink::sink_it_(const spdlog::details::log_msg& msg) {
    spdlog::memory_buf_t fmt_buf;
    spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, fmt_buf);

    std::string message;
    for (auto& c : fmt_buf) {
      message.push_back(c);
    }

    std::string display_msg = GetDisplayMsgFromMessage(message);
    message_buffer[size] = {
      .message = message,
      .display_msg = display_msg,
      .filters = GetFilters(msg.level),
      .time = std::chrono::system_clock::to_time_t(msg.time),
    };

    size++;

    if (size == capacity) {
      flush_();
    }
  }

  void EditorSink::flush_() {
    for (auto& m : message_buffer) {
      LogPanel::PushMessage(m);
    }

    size = 0;
  }

  uint32_t EditorSink::GetFilters(const spdlog::level::level_enum level) const {
    uint32_t filter = NO_FILTERS;
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

}  // namespace other
