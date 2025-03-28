/**
 * \file terminal/terminal.cpp
 **/
#include "environment/terminal.hpp"

#include <algorithm>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "memory/registers.hpp"

#include "event/event_queue.hpp"

namespace other {
  namespace {

    void BindLuaTerminal(sol::state& lua_state, Terminal* terminal);

  }  // anonymous  namespace

  Terminal::Terminal()
      : compiler(this), executor(this) {
    std::ranges::fill(input_buffer, '\0');

    command_block.command_queue = {};
    command_block.block_name = "Terminal";

    lua_state.open_libraries(sol::lib::base, sol::lib::package, sol::lib::string, sol::lib::table, sol::lib::math, sol::lib::os);
    BindLuaTerminal(lua_state, this);

    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "OtherEnv--CommandSubmitted",
      {
        std::bind_front(&Terminal::HandleEnterKey, this),
        std::bind_front(&Terminal::HandleUpDownKey, this),
      }
    );
  }

  Terminal::~Terminal() {
    Clear();
  }

  void Terminal::PushMessage(const TerminalMessage& message, bool save) {
    message_buffer.push(message);
    terminal_history.push_back(message);
    if (save) {
      stored_history.push_back(message);
    }
    history_cursor = std::nullopt;
  }

  void Terminal::PushCommand(const TerminalMessage& command) {
    RawCommand raw_command = parser.Parse(command.message);
    if (raw_command.name == "invalid") {
      PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Command not recognized : '{}'", command.message) }, false);
      return;
    }

    /// compile command to bytecode
    auto [compiled_command, arg_addresses] = compiler.Compile(raw_command);
    if (compiled_command.opcode == OpCode::INVALID_OP) {
      return;
    }

    /// queue command for dispatch
    terminal_history.push_back(command);
    stored_history.push_back(command);
    command_block.command_queue.push(compiled_command);
    for (const auto& addr : arg_addresses) {
      command_block.argument_queue.push(addr);
    }
    history_cursor = std::nullopt;
  }

  void Terminal::SourceFile(const Path& file_path) {
    switch (FNV(file_path.extension().string())) {
      case FNV(".ocmd"):
        SourceCmdFile(file_path);
        break;
      case FNV(".lua"):
        SourceLuaFile(file_path);
        break;
      case FNV(".py"):
        SourcePythonFile(file_path);
        break;

      default:
        OE_ERROR("Unrecognized file extension : {}", file_path.extension());
        PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Unrecognized file extension : '{}'", file_path.extension()) });
        break;
    }
  }

  void Terminal::Clear() {
    terminal_history.clear();
    message_buffer = {};
  }

  void Terminal::Dispatch() {
    if (command_block.command_queue.empty()) {
      return;
    }

    ExitCode ec = executor.Execute(command_block);
    if (ec != ExitCode::SUCCESS) {
      OE_ERROR("Terminal executor failed : {}", ec);
      PushMessage({ TerminalFilter::ERROR_FILTER, "Terminal executor failed" });
    }
    OE_ASSERT(command_block.command_queue.empty(), "Command queue not empty after dispatch!");
  }

  glm::vec4 Terminal::GetColorForFilter(TerminalFilter filter) const {
    if (filter & TerminalFilter::ERROR_FILTER) {
      return { 1.f, 0.f, 0.f, 1.f };
    } else if (filter & TerminalFilter::WARNING_FILTER) {
      return { 1.f, 1.f, 0.f, 1.f };
    } else if (filter & TerminalFilter::INFO_FILTER) {
      return { 0.f, 1.f, 0.f, 1.f };
    } else if (filter & TerminalFilter::DEBUG_FILTER) {
      return { 0.f, 0.f, 1.f, 1.f };
    } else {
      return { 1.f, 1.f, 1.f, 1.f };
    }
  }

  void Terminal::SourceCmdFile(const Path& file_path) {
    std::vector<RawCommand> raw_commands = parser.ParseFile(file_path);
    if (raw_commands.empty()) {
      OE_ERROR("Failed to parse file : {}", file_path.string());
      PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to parse file : '{}'", file_path.string()) });
      return;
    }

    std::vector<CommandData> compiled_commands = compiler.Compile(raw_commands);
    if (compiled_commands.empty()) {
      OE_ERROR("Failed to compile commands from file : {}", file_path.string());
      PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Failed to compile commands from file : '{}'", file_path.string()) });
      return;
    }

    for (const auto& [command, arg_addresses] : compiled_commands) {
      command_block.command_queue.push(command);
      for (const auto& addr : arg_addresses) {
        command_block.argument_queue.push(addr);
      }
    }
  }

  void Terminal::SourceLuaFile(const Path& file_path) {
    lua_state.script_file(file_path.string());
  }

  void Terminal::SourcePythonFile(const Path& file_path) {
    PushMessage({ TerminalFilter::ERROR_FILTER, "Python scripting not yet supported" });
  }

  bool Terminal::HandleEnterKey(KeyPressed& event) {
    if (event.Key() != Keyboard::Key::OE_RETURN) {
      return false;
    }

    std::string input = input_buffer.data();
    input_buffer.fill('\0');

    input.erase(input.begin(), std::find_if(input.begin(), input.end(), [](unsigned char ch) {
                  return !std::isspace(ch);
                }));
    input.erase(std::find_if(input.rbegin(), input.rend(), [](unsigned char ch) {
                  return !std::isspace(ch);
                }).base(),
                input.end());
    if (input.empty()) {
      return false;
    }

    /// TODO: retrieve active filters
    TerminalMessage message{ TerminalFilter::NO_FILTER, input };
    PushCommand(message);
    return false;
  }

  bool Terminal::HandleUpDownKey(KeyPressed& event) {
    if (event.Key() != Keyboard::Key::OE_UP && event.Key() != Keyboard::Key::OE_DOWN) {
      return false;
    }

    if (stored_history.empty()) {
      return false;
    }

    if (!history_cursor.has_value()) {
      history_cursor = stored_history.size() - 1;
    } else {
      if (event.Key() == Keyboard::Key::OE_UP) {
        if (history_cursor.value() > 0) {
          --history_cursor.value();
        }
      } else {
        if (history_cursor.value() < stored_history.size() - 1) {
          ++history_cursor.value();
        }
      }
    }

    return false;
  }

  namespace {

    void BindLuaTerminal(sol::state& lua_state, Terminal* terminal) {
      lua_state.new_enum(
        "TerminalFilter",
        "ERROR_FILTER", TerminalFilter::ERROR_FILTER,
        "WARNING_FILTER", TerminalFilter::WARNING_FILTER,
        "INFO_FILTER", TerminalFilter::INFO_FILTER,
        "DEBUG_FILTER", TerminalFilter::DEBUG_FILTER,
        "TRACE_FILTER", TerminalFilter::TRACE_FILTER,
        "BAD_FILTER", TerminalFilter::BAD_FILTER,
        "GOOD_FILTER", TerminalFilter::GOOD_FILTER,
        "TERMINAL_FILTER_ALL", TerminalFilter::TERMINAL_FILTER_ALL,
        "NUM_TERMINAL_FILTERS", TerminalFilter::NUM_TERMINAL_FILTERS
      );

      lua_state.new_usertype<TerminalMessage>(
        "TerminalMessage",
        "filters", &TerminalMessage::filters,
        "message", &TerminalMessage::message
      );

      lua_state.new_usertype<Terminal>(
        "Terminal",
        "push_message", [&](const std::string& message, TerminalFilter filter) { terminal->PushMessage({ filter, message }); },
        "push_command", &Terminal::PushCommand,
        "source_file", &Terminal::SourceFile,
        "clear", &Terminal::Clear
      );

      lua_state.set("terminal", terminal);

      // lua_state.new_usertype<CommandBlock>(
      //   "CommandBlock",
      //   "block_name", &CommandBlock::block_name,
      //   "command_queue", &CommandBlock::command_queue,
      //   "argument_queue", &CommandBlock::argument_queue
      // );

      // lua_state.new_usertype<Command>(
      //   "Command",
      //   "raw_data", &Command::data,
      //   "opcode", [](const Command& cmd) { return static_cast<OpCode>(cmd.opcode); },
      //   "num_args", [](const Command& cmd) { return cmd.num_args; }
      // );
    }

  }  // anonymous namespace

}  // namespace other