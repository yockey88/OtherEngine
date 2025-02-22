/**
 * \file parsing/command_parser.hpp
 **/
#ifndef OTHER_ENGINE_COMMAND_PARSER_HPP
#define OTHER_ENGINE_COMMAND_PARSER_HPP

#include "core/command.hpp"

#include "parsing/parser_combinators.hpp"

namespace other {

  class CommandParser {
   public:
    CommandParser();

    std::vector<RawCommand> ParseFile(const Path& file_path) const;
    RawCommand Parse(const std::string_view str) const;

   private:
    Ref<Parser<std::tuple<std::string, std::vector<std::string>>>> parser = nullptr;

    Ref<Parser<std::string>> BuildCommandNameParser();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COMMAND_PARSER_HPP