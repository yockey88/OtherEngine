/**
 * \file environment/command_executor.hpp
 **/
#ifndef OTHER_ENGINE_COMMAND_EXECUTOR_HPP
#define OTHER_ENGINE_COMMAND_EXECUTOR_HPP

#include "environment/command.hpp"
#include "environment/command_parser.hpp"

namespace other {

  enum ExecutorErrorType {
    UNKNOWN_COMMAND_CATEGORY,
    COMMAND_DOES_NOT_APPLY,

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
    Executor(Memory& memory)
        : memory(memory) {}

    ExitCode Execute(const CommandBlock& block);

   private:
    Memory& memory;

    Opt<CommandBlock> block = std::nullopt;

    ExitCode Execute(const Command& command);

    ExitCode HandleLoad(const Command& command);
    ExitCode HandleUnload(const Command& command);

    ExitCode HandleClear(const Command& command);
    ExitCode HandleExit(const Command& command);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COMMAND_EXECUTOR_HPP
