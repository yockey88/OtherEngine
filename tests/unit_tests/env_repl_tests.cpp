/**
 * \file env_repl_tests.cpp
 **/
#include <gtest.h>

#include "core/errors.hpp"
#include "environment/command.hpp"
#include "environment/command_parser.hpp"

#include "parsing/parser_combinators.hpp"

#include "oetest.hpp"

using namespace other;
class EnvReplTests : public OtherTest {
 public:
};

TEST_F(EnvReplTests, parser_components) {
  Memory memory;
  CommandParser repl_parser(memory);

  Ref<Parser<std::string>> take_command = ParseUntil(";");
  std::string command_str;

  std::string command = "load scene test_scene;";
  std::istringstream stream(command);

  ASSERT_NO_THROW(command_str = (*take_command)(stream));
  ASSERT_EQ(command_str, "load scene test_scene");

  Ref<Parser<std::vector<std::string>>> cmd_splitter = SplitStringOn(' ');
  stream = std::istringstream(command_str);
  std::vector<std::string> words{};
  ASSERT_NO_THROW(words = (*cmd_splitter)(stream));
  ASSERT_EQ(words.size(), 3u);
  ASSERT_EQ(words[0], "load");
  ASSERT_EQ(words[1], "scene");
  ASSERT_EQ(words[2], "test_scene");

  Ref<Parser<std::vector<std::string>>> cmd_parser = CollectInto(ParseUntil(";"), SplitStringOn(' '));
  stream = std::istringstream(command);
  ASSERT_NO_THROW(words = (*cmd_parser)(stream));
  ASSERT_EQ(words.size(), 3u);
  EXPECT_EQ(words[0], "load");
  EXPECT_EQ(words[1], "scene");
  ASSERT_EQ(words[2], "test_scene");

  CommandParser::CommandWithArg result = repl_parser.CreateCommand(words);

  Command cmd = result.first;
  std::string arg = result.second;

  EXPECT_EQ(cmd.category, CommandCategory::SCENE_COMMAND);
  EXPECT_EQ(cmd.command, CommandType::LOAD_COMMAND);
  EXPECT_EQ(cmd.num_args, 1);
  EXPECT_EQ(arg, "test_scene");

  CommandParser::parse_tree_t parse_tree = {
    std::nullopt,
    { words },
  };

  CommandBlock exe;
  ASSERT_NO_FATAL_FAILURE(exe = repl_parser.FinalizeCommandBlock(parse_tree));

  ASSERT_NE(exe.name, "<command-block-1>");
  ASSERT_NE(exe.command_queue.size(), 1u);
}

TEST_F(EnvReplTests, parser_components2) {
  Memory memory;
  CommandParser repl_parser(memory);

  Ref<Parser<std::string>> take_command = ParseUntil(";");
  std::string command_str;

  std::string command = "load scene test_scene";
  std::istringstream stream(command);

  ASSERT_NO_THROW(command_str = (*take_command)(stream));
  ASSERT_EQ(command_str, "load scene test_scene");

  Ref<Parser<std::vector<std::string>>> cmd_splitter = SplitStringOn(' ');
  stream = std::istringstream(command_str);
  std::vector<std::string> words{};
  ASSERT_NO_THROW(words = (*cmd_splitter)(stream));
  ASSERT_EQ(words.size(), 3u);
  ASSERT_EQ(words[0], "load");
  ASSERT_EQ(words[1], "scene");
  ASSERT_EQ(words[2], "test_scene");

  Ref<Parser<std::vector<std::string>>> cmd_parser = CollectInto(ParseUntil(";"), SplitStringOn(' '));
  stream = std::istringstream(command);
  ASSERT_NO_THROW(words = (*cmd_parser)(stream));
  ASSERT_EQ(words.size(), 3u);
  EXPECT_EQ(words[0], "load");
  EXPECT_EQ(words[1], "scene");
  ASSERT_EQ(words[2], "test_scene");

  CommandParser::CommandWithArg result = repl_parser.CreateCommand(words);

  Command cmd = result.first;
  std::string arg = result.second;

  EXPECT_EQ(cmd.category, CommandCategory::SCENE_COMMAND);
  EXPECT_EQ(cmd.command, CommandType::LOAD_COMMAND);
  EXPECT_EQ(cmd.num_args, 1);
  EXPECT_EQ(arg, "test_scene");

  CommandParser::parse_tree_t parse_tree = {
    std::nullopt,
    { words },
  };

  CommandBlock exe;
  ASSERT_NO_FATAL_FAILURE(exe = repl_parser.FinalizeCommandBlock(parse_tree));

  ASSERT_NE(exe.name, "<command-block-1>");
  ASSERT_NE(exe.command_queue.size(), 1u);
}

TEST_F(EnvReplTests, parse_single_command) {
  try {
    std::string command = "load scene test_scene;";

    Memory memory;
    CommandParser repl_parser(memory);

    Ref<Parser<CommandParser::cmd_list_t>> parse_command_list =
      Many<CommandParser::cmd_list_t>(CollectInto(SkipSpaces() >> ParseUntil(";"), SplitStringOn(' ')));

    std::istringstream stream(command);
    CommandParser::cmd_list_t commands = (*parse_command_list)(stream);
    ASSERT_EQ(commands.size(), 1u);

    CommandBlock block = repl_parser.FinalizeCommandBlock({ "test-block", commands });
    ASSERT_EQ(block.name, "test-block");
    ASSERT_EQ(block.command_queue.size(), 2u);

    ASSERT_NO_FATAL_FAILURE(block = repl_parser.ParseBlock(command));

    /// LOAD_SCENE command
    /// address for argument in 'memory'
    ASSERT_EQ(block.name, "<command-block-1>");
    ASSERT_EQ(block.command_queue.size(), 2u);

    Command expected_cmd = block.command_queue.front();
    block.command_queue.pop();

    ASSERT_EQ(expected_cmd.priority, other::DEFAULT_COMMAND_PRIORITY);
    ASSERT_EQ(expected_cmd.category, other::SCENE_COMMAND);
    ASSERT_EQ(expected_cmd.command, other::LOAD_COMMAND);
    ASSERT_EQ(expected_cmd.num_args, 1u);

    Command argument = block.command_queue.front();
    block.command_queue.pop();

    std::string scene_name = "";
    ASSERT_NO_THROW(scene_name = memory.GetString(argument.argument_address));
    ASSERT_EQ(scene_name, "test_scene");
  } catch (other::ParsingError) {
    FAIL() << "Parsing Error\n";
  }
}

TEST_F(EnvReplTests, parse_multiple_commands) {
  try {
    std::string command = "load scene test_scene;\nload scene test_scene2;";

    Memory memory;
    CommandParser repl_parser(memory);

    Ref<Parser<CommandParser::cmd_list_t>> parse_command_list =
      Many<CommandParser::cmd_list_t>(CollectInto(SkipSpaces() >> ParseUntil(";"), SplitStringOn(' ')));

    std::istringstream stream(command);
    CommandParser::cmd_list_t commands = (*parse_command_list)(stream);
    ASSERT_EQ(commands.size(), 2u);

    CommandBlock block = repl_parser.FinalizeCommandBlock({ "test-block", commands });
    ASSERT_EQ(block.name, "test-block");
    ASSERT_EQ(block.command_queue.size(), 4u);

    ASSERT_NO_FATAL_FAILURE(block = repl_parser.ParseBlock(command));

    /// LOAD_SCENE command
    /// address for argument in 'memory'
    ASSERT_EQ(block.name, "<command-block-1>");
    ASSERT_EQ(block.command_queue.size(), 4u);

    Command expected_cmd = block.command_queue.front();
    block.command_queue.pop();

    ASSERT_EQ(expected_cmd.priority, other::DEFAULT_COMMAND_PRIORITY);
    ASSERT_EQ(expected_cmd.category, other::SCENE_COMMAND);
    ASSERT_EQ(expected_cmd.command, other::LOAD_COMMAND);
    ASSERT_EQ(expected_cmd.num_args, 1u);

    Command argument = block.command_queue.front();
    block.command_queue.pop();

    std::string scene_name = "";
    ASSERT_NO_THROW(scene_name = memory.GetString(argument.argument_address));
    ASSERT_EQ(scene_name, "test_scene");

    expected_cmd = block.command_queue.front();
    block.command_queue.pop();

    ASSERT_EQ(expected_cmd.priority, other::DEFAULT_COMMAND_PRIORITY);
    ASSERT_EQ(expected_cmd.category, other::SCENE_COMMAND);
    ASSERT_EQ(expected_cmd.command, other::LOAD_COMMAND);
    ASSERT_EQ(expected_cmd.num_args, 1u);

    argument = block.command_queue.front();
    block.command_queue.pop();

    scene_name = "";
    ASSERT_NO_THROW(scene_name = memory.GetString(argument.argument_address));
    ASSERT_EQ(scene_name, "test_scene2");
  } catch (other::ParsingError) {
    FAIL() << "Parsing Error\n";
  }
}
