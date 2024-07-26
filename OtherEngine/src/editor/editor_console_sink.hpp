/**
 * \file editor/console_editor_sink.hpp
 **/
#ifndef OTHER_ENGINE_EDITOR_CONSOLE_SINK_HPP
#define OTHER_ENGINE_EDITOR_CONSOLE_SINK_HPP

#include <spdlog/common.h>
#include <spdlog/sinks/base_sink.h>

#include "core/defines.hpp"

namespace other {

  enum ConsoleMessageFilters : uint8_t {
    NO_FILTERS = 0 ,
    DEBUG_FILTER   = bit(2) , 
    INFO_FILTER    = bit(3) ,
    WARNING_FILTER = bit(4) ,
    ERR_FILTER     = bit(5) ,
    
    DEFAULT_FILTER = INFO_FILTER | WARNING_FILTER | ERR_FILTER ,

    /// dont filter errors
    ALL = DEBUG_FILTER  | INFO_FILTER | WARNING_FILTER | ERR_FILTER ,
  };

  struct ConsoleMessage {
    std::string message;
    std::string display_msg;

    uint8_t filters = NO_FILTERS;
    time_t time;
  };

  class EditorConsoleSink : public spdlog::sinks::base_sink<std::mutex> {
    public:
      EditorConsoleSink(size_t capacity) 
          : capacity(capacity) , message_buffer(capacity) {}
      virtual ~EditorConsoleSink() override {}

      EditorConsoleSink(const EditorConsoleSink&) = delete;
      EditorConsoleSink& operator=(const EditorConsoleSink&) = delete;

    private:
      size_t capacity = 0;
      std::vector<ConsoleMessage> message_buffer{};

      size_t size = 0;

      void sink_it_(const spdlog::details::log_msg& msg) override;
      void flush_() override;

      uint8_t GetFilters(const spdlog::level::level_enum level) const;
  };

} // namespace other

#endif // !OTHER_ENGINE_CONSOLE_SINK_HPP
