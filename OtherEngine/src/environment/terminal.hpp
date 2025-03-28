/**
 * \file terminal/terminal.hpp
 **/
#ifndef OTHER_ENGINE_TERMINAL_TERMINAL_HPP
#define OTHER_ENGINE_TERMINAL_TERMINAL_HPP

#include <array>
#include <queue>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <sol/sol.hpp>

#include "environment/command_compiler.hpp"
#include "environment/command_executor.hpp"

#include "event/key_events.hpp"
#include "parsing/command_parser.hpp"

namespace other {

  enum TerminalFilter : uint16_t {
    NO_FILTER = 0,

    ERROR_FILTER = bit(0),
    WARNING_FILTER = bit(1),
    INFO_FILTER = bit(2),
    DEBUG_FILTER = bit(3),
    TRACE_FILTER = bit(4),

    BAD_FILTER = ERROR_FILTER | WARNING_FILTER,
    GOOD_FILTER = INFO_FILTER | DEBUG_FILTER | TRACE_FILTER,
    TERMINAL_FILTER_ALL = ERROR_FILTER | WARNING_FILTER | INFO_FILTER | DEBUG_FILTER | TRACE_FILTER,

    NUM_TERMINAL_FILTERS,
    INVALID_TERMINAL_FILTER = NUM_TERMINAL_FILTERS
  };

  struct TerminalMessage {
    TerminalFilter filters;
    std::string message;
  };

  class Terminal {
   public:
    Terminal();
    ~Terminal();

    void PushMessage(const TerminalMessage& message, bool save = true);
    void PushCommand(const TerminalMessage& command);

    void SourceFile(const Path& file_path);

    void Clear();

    void Dispatch();

    glm::vec4 GetColorForFilter(TerminalFilter filter) const;

   private:
    friend struct Environment;
    friend class CommandExecutor;

    static constexpr size_t kInputBufferSize = 1024;
    std::array<char, kInputBufferSize> input_buffer;

    Opt<uint32_t> history_cursor = std::nullopt;
    std::vector<TerminalMessage> terminal_history;
    std::vector<TerminalMessage> stored_history;
    std::queue<TerminalMessage> message_buffer;

    sol::state lua_state;
    CommandParser parser;
    CommandCompiler compiler;
    CommandExecutor executor;

    CommandBlock command_block;

    void SourceCmdFile(const Path& file_path);
    void SourceLuaFile(const Path& file_path);
    void SourcePythonFile(const Path& file_path);

    bool HandleEnterKey(KeyPressed& event);
    bool HandleUpDownKey(KeyPressed& event);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_TERMINAL_TERMINAL_HPP