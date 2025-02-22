/**
 * \file parsing/command_parser.cpp
 **/
#include "parsing/command_parser.hpp"

#include <fstream>

#include "core/logger.hpp"

namespace other {

  CommandParser::CommandParser() {
    auto command_name = SkipSpaces() >> BuildCommandNameParser();
    auto args = SkipSpaces() >> Many<std::vector<std::string>>(SkipSpaces() >> MatchAnyWord());
    parser = ParseMultiple(command_name, args);
    OE_ASSERT(parser != nullptr, "Parser is null");
  }

  std::vector<RawCommand> CommandParser::ParseFile(const Path& file_path) const {
    std::ifstream file{ file_path };
    if (!file.is_open()) {
      OE_ERROR("Failed to open file : {}", file_path.string());
      return {};
    }

    std::vector<RawCommand> commands;
    std::string line;
    while (std::getline(file, line)) {
      commands.push_back(Parse(line));
    }

    return commands;
  }

  RawCommand CommandParser::Parse(const std::string_view str) const {
    std::istringstream stream{ std::string{ str } };

    std::tuple<std::string, std::vector<std::string>> result;
    try {
      result = (*parser)(stream);
    } catch (const ParsingError& e) {
      return RawCommand{
        .name = "invalid",
      };
    }

    auto [name, args] = result;
    return RawCommand{
      .name = name,
      .args = args,
    };
  }

  Ref<Parser<std::string>> CommandParser::BuildCommandNameParser() {
    std::vector<std::string> command_names;
    for (const auto& cmd : kAvailableCommands) {
      command_names.push_back(std::string{ cmd.name });
    }

    Ref<Parser<std::string>> parser = MatchAnyStringFrom(command_names);
    return parser;
  }

}  // namespace other