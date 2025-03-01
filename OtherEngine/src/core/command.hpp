/**
 * \file core/command.hpp
 **/
#ifndef OTHER_ENGINE_CORE_COMMAND_HPP
#define OTHER_ENGINE_CORE_COMMAND_HPP

#include <array>
#include <queue>
#include <string>
#include <vector>

#include "memory/registers.hpp"

namespace other {

  /// these are the built in core commands for the other environment terminal, if
  ///     a command fails to match any of these then there are a few steps that will happen to attempt to match the command
  ///     to any user defined commands
  ///   1 - attempt to match command against a lua function of the same name with the same arity
  ///   2 - attempt to match command against a python function of the same name with the same arity
  ///   3 - attempt pass command to a loaded module for handling if the module has the 'HandleCommand' function implemented and successfully loaded

  struct RawCommand {
    std::string name;
    std::vector<std::string> args;
  };

  /**
   * Command Categories
   *
   *  - control commands : commands for controlling the terminal
   *      ex: help, clear, exit, call, lua, py
   *  - file commands : commands for interacting with the filesystem
   *      ex: ls, pwd, source
   *  - module commands : commands for interacting with modules
   *      ex: load, unload
   *  - net commands : commands for networking
   *      ex: listen, connect
   *  - debug commands : commands for debugging
   *      ex: echo
   *  - other commands : commands that don't fit into the above categories
   *      ex: ...
   **/

  enum CommandCategory : uint8_t {
    CONTROL_CMD = 0x01,
    FILE_CMD = 0x02,
    MODULE_CMD = 0x03,
    DEBUG_CMD = 0x04,
    NET_CMD = 0x05,
    OTHER_CMD = 0xFF
  };

  enum ControlCommands : uint8_t {
    HELP_CMD = 0x01,
    CLEAR_CMD = 0x02,
    EXIT_CMD = 0x03,
    CALL_CMD = 0x04,
    LUA_CALL_CMD = 0x05
  };

  enum FileCommands : uint8_t {
    LS_CMD = 0x01,
    PWD_CMD = 0x02,
    SOURCE_CMD = 0x03,
    MOUNT_CMD = 0x04,
  };

  enum ModuleCommands : uint8_t {
    LOAD_CMD = 0x01
  };

  enum NetCommands : uint8_t {
    LISTEN_CMD = 0x01,
    CONNECT_CMD = 0x02
  };

  enum DebugCommands : uint8_t {
    ECHO_CMD = 0x01
  };

  enum OtherCommands : uint8_t {
    CREATE_CMD = 0x01
  };

#define COMMAND(cat, val) (cat << 8 | val)
#define CATEGORY(op) ((op & 0xFF00) >> 8)
#define VALUE(op) (op & 0x00FF)

  enum OpCode : uint16_t {
    NO_OP = 0,

    // control commands
    HELP_OP = COMMAND(CONTROL_CMD, HELP_CMD),
    CLEAR_OP = COMMAND(CONTROL_CMD, CLEAR_CMD),
    EXIT_OP = COMMAND(CONTROL_CMD, EXIT_CMD),
    CALL_OP = COMMAND(CONTROL_CMD, CALL_CMD),
    LUA_CALL_OP = COMMAND(CONTROL_CMD, LUA_CALL_CMD),

    /// file commands
    LS_OP = COMMAND(FILE_CMD, LS_CMD),
    PWD_OP = COMMAND(FILE_CMD, PWD_CMD),
    SOURCE_OP = COMMAND(FILE_CMD, SOURCE_CMD),
    MOUNT_OP = COMMAND(FILE_CMD, MOUNT_CMD),

    /// module commands
    LOAD_OP = COMMAND(MODULE_CMD, LOAD_CMD),

    /// net commands
    LISTEN_OP = COMMAND(NET_CMD, LISTEN_CMD),
    CONNECT_OP = COMMAND(NET_CMD, CONNECT_CMD),

    /// debug commands
    ECHO_OP = COMMAND(DEBUG_CMD, ECHO_CMD),

    /// other commands
    CREATE_OP = COMMAND(OTHER_CMD, CREATE_CMD),

    NUM_OPS,
    INVALID_OP = NUM_OPS
  };

  struct ArityRange {
    uint16_t min;
    uint16_t max;

    constexpr ArityRange(uint16_t arity) : min(arity), max(arity) {}
    constexpr ArityRange(uint16_t min, uint16_t max) : min(min), max(max) {}
    constexpr auto operator<=>(const ArityRange&) const = default;
  };

  struct OtherCommand {
    OpCode opcode;

    std::string_view name;
    uint64_t hash;

    ArityRange num_args;

    std::string_view description;
    std::string_view meta_var_name;

    constexpr OtherCommand(OpCode code, const std::string_view name, uint16_t num_args, const std::string_view description, const std::string_view meta_var_name)
        : opcode(code), name(name), hash(FNV(name)), num_args(num_args), description(description), meta_var_name(meta_var_name) {}
    constexpr OtherCommand(OpCode code, const std::string_view name, uint16_t min_args, uint16_t max_args, const std::string_view description, const std::string_view meta_var_name)
        : opcode(code), name(name), hash(FNV(name)), num_args(min_args, max_args), description(description), meta_var_name(meta_var_name) {}

    constexpr auto operator<=>(const OtherCommand&) const = default;
  };

  constexpr static std::array kAvailableCommands = {
    /// control commands
    OtherCommand{ OpCode::HELP_OP, "help", 0, "Print the help menu", "" },
    OtherCommand{ OpCode::CLEAR_OP, "clear", 0, "Clear the terminal screen", "" },
    OtherCommand{ OpCode::EXIT_OP, "exit", 0, "Exit the terminal", "" },
    OtherCommand{ OpCode::CALL_OP, "call", 1, 256, "Call a function", "function" },
    OtherCommand{ OpCode::LUA_CALL_OP, "lua", 1, 256, "Call a lua function", "function" },

    /// file commands
    OtherCommand{ OpCode::LS_OP, "ls", 0, "Lists the mounted directories", "directory" },
    OtherCommand{ OpCode::PWD_OP, "pwd", 0, "Print the current working directory", "" },
    OtherCommand{ OpCode::SOURCE_OP, "source", 1, "Source a file", "file" },
    OtherCommand{ OpCode::MOUNT_OP, "mount", 1, "Mount a directory", "directory" },

    /// module commands
    OtherCommand{ OpCode::LOAD_OP, "load", 1, "Load a module", "module" },

    /// net commands
    OtherCommand{ OpCode::LISTEN_OP, "listen", 1, "Listen for incoming connections", "port" },
    OtherCommand{ OpCode::CONNECT_OP, "connect", 1, "Connect to a remote host", "host" },

    /// debug commands
    OtherCommand{ OpCode::ECHO_OP, "echo", 1, "Print a message", "message" },

    /// other commands
    OtherCommand{ OpCode::CREATE_OP, "create", 1, "Create a new object", "object" }
  };

  /**
   * command bytecode
   *     - args are stored in registers and their addresses are indexed into by the value in the corresponding arg field
   * TODO: how to better handle arguments st we are not limited by the number of registers?
   *       (we could implement a custom arbitrary size integer and determine how many bits to parse using the num_args field)
   * |--------------------|
   * | 16 bits | 16 bits  |
   * |--------------------|
   * |  opcode | num-args |
   * |--------------------|
   **/
  struct Command {
    union {
      uint32_t data = 0;
      struct {
        uint16_t opcode;
        uint16_t num_args;
      };
    };
  };

  struct CommandBlock {
    std::string block_name = "< CommandBlock >";
    std::queue<Command> command_queue;
    std::queue<address_t> argument_queue;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_CORE_COMMAND_HPP