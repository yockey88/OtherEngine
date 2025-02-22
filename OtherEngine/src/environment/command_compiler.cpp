/**
 * \file terminal/command_compiler.cpp
 **/
#include "environment/command_compiler.hpp"

#include "core/logger.hpp"
#include "environment/terminal.hpp"

namespace other {

  CommandCompiler::CommandCompiler(Terminal* term)
      : terminal(term) {
    OE_ASSERT(terminal != nullptr, "Terminal is null!");
  }

  std::vector<CommandData> CommandCompiler::Compile(const std::vector<RawCommand>& raw_commands) {
    std::vector<CommandData> compiled_commands;
    compiled_commands.reserve(raw_commands.size());

    for (auto itr = raw_commands.begin(); itr != raw_commands.end(); ++itr) {
      CommandData compiled_command = Compile(*itr);
      compiled_commands.push_back(compiled_command);
    }

    return compiled_commands;
  }

  CommandData CommandCompiler::Compile(const RawCommand& raw_command) {
    OpCode opcode = OpCode::INVALID_OP;
    uint64_t name_hash = FNV(raw_command.name);
    for (auto& cmd : kAvailableCommands) {
      if (cmd.hash == name_hash) {
        opcode = cmd.opcode;
      }
    }

    if (opcode == OpCode::INVALID_OP) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Command not recognized : '{}'", raw_command.name) }, false);
      return { Command{ OpCode::INVALID_OP }, {} };
    }

    auto itr = std::ranges::find_if(kAvailableCommands, [opcode](const OtherCommand& cmd) { return cmd.opcode == opcode; });
    /// this is impossible because we already checked for invalid opcode
    OE_ASSERT(itr != kAvailableCommands.end(), "Failed to find command in available commands");

    if (raw_command.args.size() < itr->num_args.min || raw_command.args.size() > itr->num_args.max) {
      terminal->PushMessage({ TerminalFilter::ERROR_FILTER, fmtstr("Invalid number of arguments for command : '{}'", raw_command.name) }, false);
      return { Command{ OpCode::INVALID_OP }, {} };
    }

    Command command;
    command.opcode = opcode;
    command.num_args = raw_command.args.size();

    std::vector<address_t> arg_addresses;
    arg_addresses.reserve(command.num_args);

    Registers& registers = Arena::GetRegisters();
    for (auto itr = raw_command.args.begin(); itr != raw_command.args.end(); ++itr) {
      address_t addr = registers.Write(*itr);
      OE_ASSERT(addr != Registers::null_address, "Failed to write argument to register");
      arg_addresses.push_back(addr);
    }

    return { command, arg_addresses };
  }

}  // namespace other