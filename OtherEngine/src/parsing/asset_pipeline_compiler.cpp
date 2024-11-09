/**
 * \file parsing/asset_pipeline_compiler.cpp
 **/
#include "parsing/asset_pipeline_compiler.hpp"

#include <ranges>
#include <sstream>
#include <string>

#include "core/errors.hpp"
#include "core/logger.hpp"


namespace other {
  /// <command> <command> .... <command>
  /// <command> ::= "from" <instruction> | "to" <resource> <instruction>
  /// <instruction> ::= <resource> <action>
  /// <resource> ::= <hash>
  /// <action> ::= <hash>
  /// <hash> ::= uint64_t

  AssetPipeline AssetPipelineCompiler::CompileSource(const std::string_view src) {
    source = src;

    std::string line;
    std::istringstream ss{ source };
    while (std::getline(ss, line)) {
      pipeline.AddCommand(ParseCommand(line));
    }

    return pipeline;
  }

  AssetPipeline::Command AssetPipelineCompiler::ParseCommand(const std::string_view line) {
    using namespace std::string_view_literals;

    tokens = line | std::views::split(' ') | std::ranges::to<std::vector<std::string>>();
    /// must have
    ///   - command keyword (from, to, etc..)
    ///   - resource to act on
    ///   - action to perform on resource
    if (tokens.size() < 3) {
      std::string err_msg;
      if (tokens.empty()) {
        err_msg = "Invalid Command, command is empty";
      } else {
        err_msg = "Invalid Command, command is too short";
      }
      throw AssetPipelineCompilerError(err_msg, AssetPipelineCompilerErrorType::INVALID_COMMAND_KEYWORD);
    }

    AssetPipeline::Command command;
    command.type = tokens[0] == "from"sv ?
      AssetPipeline::CommandType::FROM_COMMAND :
      AssetPipeline::CommandType::TO_COMMAND;

    command.instruction.resource_hash = pipeline.AddResource(tokens[1]);

    std::vector<std::string> action_tokens(tokens.begin() + 2, tokens.end());
    command.instruction.action_hash = ParseAction(action_tokens, tokens);

    return command;
  }

  uint64_t AssetPipelineCompiler::ParseAction(std::vector<std::string>& action_tokens, const std::vector<std::string>& tokens) {
    using namespace std::string_literals;

    if (action_tokens.size() < 2) {
      throw AssetPipelineCompilerError("Invalid Action, action is too short", AssetPipelineCompilerErrorType::INVALID_COMMAND_KEYWORD);
    }

    /// load, write, etc...
    std::string action_token = action_tokens[0];
    /// any extra parameters
    std::vector<std::string> raw_parameters(action_tokens.begin() + 1, action_tokens.end());

    return 0;
  }

  bool AssetPipelineCompiler::AtEnd() const {
    return index >= source.size();
  }

  char AssetPipelineCompiler::Peek() const {
    if (AtEnd()) {
      return '\0';
    }
    return source[index];
  }

}  // namespace other