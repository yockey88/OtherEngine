/**
 * \file environment/command_executor.hpp
 **/
#ifndef OTHER_ENGINE_COMMAND_EXECUTOR_HPP
#define OTHER_ENGINE_COMMAND_EXECUTOR_HPP

#include "environment/command_parser.hpp"
#include "environment/command.hpp"

namespace other {

  class Executor;

  struct Instructions {
    std::string module_name = "";
    std::map<uint64_t, CommandBlock> blocks;

    ExitCode Execute(Executor& exe);
    ExitCode Execute(Executor& exe, const uint64_t block_hash);
    ExitCode Execute(Executor& exe, const std::string_view block_name);
    ExitCode Execute(Executor& exe, CommandBlock& block);
  };

  enum ExecutorErrorType {
    COMMAND_MISSING_ARGUMENT,
    COMMAND_INVALID_ARGUMENT,
  };

  struct ExecutorError {
    ExecutorErrorType type;

    ExecutorError(ExecutorErrorType type)
      : type(type) {}
  };

  class Executor {
    public:
      Executor(Memory& memory, const Instructions& inst_set) 
          : memory(memory), instruction_set(inst_set) {}

      ExitCode Execute();
      ExitCode Execute(const uint64_t hash);
      ExitCode Execute(const std::string_view block);
      ExitCode Execute(CommandBlock& block);

      void Execute(CommandType command, CommandCategory category, CommandPriority prio, Opt<address_t> address = std::nullopt);

    private:
      Memory& memory;
      Instructions instruction_set;


      void HandleSceneCommand(CommandType command, Opt<address_t> address);
  };

} // namespace other

#endif // !OTHER_ENGINE_COMMAND_EXECUTOR_HPP
