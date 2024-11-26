/**
 * \file environment/terminal.hpp
 **/
#ifndef OTHER_ENGINE_TERMINAL_HPP
#define OTHER_ENGINE_TERMINAL_HPP

#include <queue>
#include <string>

#include "core/defines.hpp"
#include "environment/command.hpp"
#include "environment/command_executor.hpp"
#include "environment/command_parser.hpp"
#include "environment/memory.hpp"

namespace other {

  enum class TerminalFilters : uint32_t {
    NO_FILTERS = 0,
    DEBUG_FILTER = bit(2),
    INFO_FILTER = bit(3),
    WARNING_FILTER = bit(4),
    ERR_FILTER = bit(5),

    COMMAND_FILTER = bit(6),
    INVALID_COMMAND_FILTER = COMMAND_FILTER | ERR_FILTER,

    DEFAULT_FILTER = INFO_FILTER | WARNING_FILTER | ERR_FILTER,
  };

  struct TerminalMessage {
    std::string message;
    TerminalFilters filters = TerminalFilters::NO_FILTERS;
  };

  class Terminal {
   public:
    Terminal(Memory& memory)
        : memory(memory), parser(memory), executor(memory) {}
    ~Terminal() = default;

    void PushMessage(const TerminalMessage& message);
    void PushCommand(const Command& command);

    void ClearMessages();
    void ClearCommands();

    void Dispatch();

   private:
    static constexpr size_t kInputBufferSize = 1024;
    static std::array<char, kInputBufferSize> input_buffer;

    std::vector<TerminalMessage> terminal_history;
    std::queue<TerminalMessage> message_buffer;

    Memory& memory;
    CommandParser parser;

    CommandBlock command_block{ .name = "<terminal-command>" };
    Executor executor;

    friend struct Environment;
    friend class Executor;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_TERMINAL_HPP
