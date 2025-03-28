/**
 * \file terminal/command_compiler.hpp
 **/
#ifndef OTHER_ENGINE_TERMINAL_COMMAND_COMPILER_HPP
#define OTHER_ENGINE_TERMINAL_COMMAND_COMPILER_HPP

#include "core/command.hpp"
#include "memory/registers.hpp"


namespace other {

  using CommandData = std::pair<Command, std::vector<address_t>>;

  class Terminal;

  class CommandCompiler {
   public:
    CommandCompiler(Terminal* term);
    ~CommandCompiler() = default;

    std::vector<CommandData> Compile(const std::vector<RawCommand>& raw_commands);
    CommandData Compile(const RawCommand& raw_command);

   private:
    Terminal* terminal = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_TERMINAL_COMMAND_COMPILER_HPP