/**
 * \file asset/asset_pipeline.cpp
 **/
#include "asset/asset_pipeline.hpp"

namespace other {

  void AssetPipeline::AddCommand(const Command& command) {
    commands.push_back(command);
  }

  void AssetPipeline::AddCommand(const CommandType type, const Instruction& instruction) {
    Command command;
    command.type = type;
    command.instruction = instruction;
    commands.push_back(command);
  }

  uint64_t AssetPipeline::AddResource(const std::string_view path) {
    uint64_t hash = FNV(path);
    if (resource_names.find(hash) == resource_names.end()) {
      resource_names[hash] = path;
    }
    return hash;
  }

  bool AssetPipeline::HasResource(const std::string_view path) const {
    return resource_names.find(FNV(path)) != resource_names.end();
  }

  bool AssetPipeline::HasResource(const uint64_t hash) const {
    return resource_names.find(hash) != resource_names.end();
  }

  const std::map<uint64_t, std::string>& AssetPipeline::GetResources() const {
    return resource_names;
  }

  void AssetPipeline::Execute() {
    for (auto& command : commands) {
      /// TODO: implement
      /// - look up resource using hash
      /// - look up action using hash
      /// - validate resource and action
      /// - execute action on resource
      ///
      /// maybe:
      // switch (command.type) {
      //   case CommandType::FROM_COMMAND:
      //     ProcessFromCommand(command);
      //     break;
      //   case CommandType::TO_COMMAND:
      //     ProcessToCommand(command);
      //     break;
      //   default:
      //     break;
      // }
    }
  }

}  // namespace other