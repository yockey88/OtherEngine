/**
 * \file environment/command_parser.hpp
 **/
#ifndef OTHER_ENGINE_COMMAND_PARSER_HPP
#define OTHER_ENGINE_COMMAND_PARSER_HPP

#include <queue>

#include "environment/command.hpp"
#include "environment/memory.hpp"

namespace other {

  /**
   *
   * command grammar:
   *  <cmd> := <priority>? <command> <category> (<args>*)?
   *  <cmd-scope> := '[' <identifier> ']' '::' '{' <cmd> '}'
   *
   **/

  struct CommandBlock {
    std::string name = "<no-op block>";
    std::queue<Command> command_queue; 
  };

  class CommandParser {
    public:
      CommandParser(Memory& memory)
        : memory(memory) {}
      ~CommandParser() = default;

      using CommandWithArg = std::pair<Command, std::string>;
      using parsed_cmd_t = std::vector<std::string>;
      using cmd_list_t = std::vector<parsed_cmd_t>;
      /// name?, '::'?, '{'?, cmd1, cmd2, ..., cmdn, '}'?
      using parse_tree_t = std::tuple<Opt<std::string>,cmd_list_t>;

      CommandWithArg CreateCommand(const parsed_cmd_t& command);
      CommandBlock FinalizeCommandBlock(const parse_tree_t& ir);
      CommandBlock ParseBlock(const std::string_view block);

    private:
      size_t block_num = 0;
      Memory& memory;
  };

} // namespace other

#endif // !OTHER_ENGINE_COMMAND_PARSER_HPP
