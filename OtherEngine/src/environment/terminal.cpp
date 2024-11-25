/**
 * \file environment/terminal.cpp
 **/
#include "environment/terminal.hpp"

#include "editor/editor_sink.hpp"
#include "editor/panels/log_panel.hpp"
#include "environment/command.hpp"
#include "environment/command_parser.hpp"

#include "application/app_state.hpp"

namespace other {

  std::array<char, Terminal::kInputBufferSize> Terminal::input_buffer;

  void Terminal::PushMessage(const TerminalMessage& message) {
    OE_TRACE("PUSHING MESSAGE filters = {}", message.filters);
    message_buffer.push(message);
    terminal_history.push_back(message);
  }

  void Terminal::PushCommand(const Command& command) {
    OE_TRACE("PUSHING COMMAND : ({}) {}:{} [ {} ]", command.priority, command.category, command.command, command.num_args);
    command_block.command_queue.push(command);
  }

  void Terminal::ClearMessages() {
    message_buffer = {};
  }

  void Terminal::Dispatch() {
    while (!message_buffer.empty()) {
      CommandBlock command = parser.ParseBlock(message_buffer.front().message);
      message_buffer.pop();

      while (!command.command_queue.empty()) {
        command_block.command_queue.push(command.command_queue.front());
        command.command_queue.pop();
      }
    }

    ExitCode ec = executor.Execute(command_block);
    if (ec != ExitCode::SUCCESS) {
      OE_ERROR("Terminal executor failed : {}", ec);
    }
    command_block = {};
  }

}  // namespace other