/**
 * \file environment/command_parser.cpp
 **/
#include "environment/command_parser.hpp"

#include "core/formatters.hpp"
#include "environment/command.hpp"

#include "parsing/parser_combinators.hpp"

namespace other {

  CommandParser::CommandWithArg CommandParser::CreateCommand(const parsed_cmd_t& command) {
    if (command.empty()) {
      OE_ERROR("INVALID command! : {}", fmt::join(command, " , "));
      Command cmd;
      cmd.priority = DEFAULT_COMMAND_PRIORITY;
      cmd.category = EMPTY_COMMAND;
      cmd.command = NO_OP_COMMAND;
      cmd.num_args = 0;
      return { cmd, "" };
    } else if (command.size() == 1) {
      uint64_t hash = FNV(command[0]);
      auto command_itr = std::ranges::find_if(CommandMap::kCommandType, [&](const CommandMap::TypePair& pair) -> bool { return pair.first == hash; });

      if (command_itr == CommandMap::kCommandType.end()) {
        OE_ERROR("Unknown Command : {}", command[0]);
        return CommandWithArg{ {}, {} };
      }

      Command cmd;
      cmd.priority = DEFAULT_COMMAND_PRIORITY;
      cmd.category = CommandCategory::ENVIRONMENT_COMMAND;
      cmd.command = command_itr->second;
      cmd.num_args = 0;
      return CommandWithArg{ cmd, "" };
    }

    uint64_t hash_1 = FNV(command[0]);
    uint64_t hash_2 = FNV(command[1]);

    auto command_itr = std::ranges::find_if(CommandMap::kCommandType, [&](const CommandMap::TypePair& pair) -> bool { return pair.first == hash_1; });
    auto category = std::ranges::find_if(CommandMap::kCommandCategory, [&](const CommandMap::CategoryPair& pair) -> bool { return pair.first == hash_2; });

    if (category == CommandMap::kCommandCategory.end()) {
      OE_ERROR("Unknown command category : {}", command[0]);
      return CommandWithArg{ {}, {} };
    }
    if (command_itr == CommandMap::kCommandType.end()) {
      OE_ERROR("Unknown command command : {}", command[1]);
      return CommandWithArg{ {}, {} };
    }

    Command cmd;
    cmd.priority = DEFAULT_COMMAND_PRIORITY;
    cmd.category = category->second;
    cmd.command = command_itr->second;
    cmd.num_args = command.size() >= 3 ? 1 : 0;

    CommandWithArg result = { cmd, "" };

    if (command.size() >= 3) {
      OE_DEBUG("COMMAND : {}:{} [ {} ]", command[0], command[1], command[2]);
      result.second = command[2];
    } else {
      OE_DEBUG("COMMAND : {}:{}", command[0], command[1]);
    }

    return result;
  }

  CommandBlock CommandParser::ParseCommand(const std::string_view message) {
    auto parser = CollectInto(SkipSpaces() >> ParseUntil(";"), SplitStringOn(' '));
    std::istringstream stream{ std::string{ message } };
    parsed_cmd_t words = (*parser)(stream);

    CommandWithArg result = CreateCommand(words);
    CommandBlock block;
    block.command_queue.push(result.first);

    if (result.second != "") {
      address_t arg_address = memory.Alloc(result.second);
      Command argument;
      argument.argument_address = arg_address;
      block.command_queue.push(argument);
    }

    return block;
  }

  CommandBlock CommandParser::FinalizeCommandBlock(const parse_tree_t& ir) {
    auto [id_opt, cmds] = ir;
    if (cmds.empty()) {
      OE_WARN("Parsing empty command list! : {}", id_opt.has_value() ? *id_opt : "<nameless-block>");
      return {};
    }

    CommandBlock res;
    if (id_opt.has_value()) {
      res.name = *id_opt;
    } else {
      res.name = fmtstr("<command-block-{}>", block_num);
    }
    ++block_num;

    for (auto& cwa : cmds) {
      CommandWithArg parse_command = CreateCommand(cwa);
      res.command_queue.push(parse_command.first);

      if (parse_command.second != "") {
        address_t arg_address = memory.Alloc(parse_command.second);
        Command argument;
        argument.argument_address = arg_address;
        res.command_queue.push(argument);
      }
    }

    return res;
  }

  CommandBlock CommandParser::ParseBlock(const std::string_view block) {
    try {
      std::string block_str = TrimBeginningAndEnd(std::string{ block });
      std::istringstream stream(block_str);

      Ref<Parser<std::string>> name_parser = MatchIdentifierAndStripParens();
      Ref<Parser<void>> skip_chars = SkipSpaces() >> Skip(GroupMatcher("{::}"));
      Ref<Parser<cmd_list_t>> parse_command_list = Many<cmd_list_t>(CollectInto(ParseUntil(";"), SplitStringOn(' ')));

      Opt<std::string> name;
      cmd_list_t cmds;

      if (block_str.starts_with('[')) {
        name = (*name_parser)(stream);

        (*skip_chars)(stream);
        (*skip_chars)(stream);
        (*skip_chars)(stream);
      }

      cmds = (*parse_command_list)(stream);

      if (stream.peek() == '}') {
        (*skip_chars)(stream);
      }

      return FinalizeCommandBlock({ name, cmds });

    } catch (...) {
      return { "", {} };
    }
  }

}  // namespace other
