/**
 * \file editor/editor_sink.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_SINK_HPP
#define OTHER_ENGINE_EDITOR_SINK_HPP

#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>

#include "core/defines.hpp"
#include "environment/terminal.hpp"

namespace other {

  std::string GetDisplayMsgFromMessage(const std::string& message);

  struct ConsoleMessage {
    std::string message;
    std::string display_msg;

    uint32_t filters = TerminalFilter::NO_FILTER;
    time_t time;
  };

  constexpr inline size_t kDefaultBufferSize = 1024;

  class EditorSink : public spdlog::sinks::base_sink<std::mutex> {
   public:
    EditorSink(size_t capacity = kDefaultBufferSize)
        : capacity(capacity), message_buffer(capacity) {}
    virtual ~EditorSink() override {}

    EditorSink(const EditorSink&) = delete;
    EditorSink& operator=(const EditorSink&) = delete;

   private:
    size_t capacity = 0;
    std::vector<ConsoleMessage> message_buffer{};

    size_t size = 0;

    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

    uint32_t GetFilters(const spdlog::level::level_enum level) const;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SINK_HPP
