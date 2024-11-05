/**
 * \file asset/asset_pipeline.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_PIPELINE_HPP
#define OTHER_ENGINE_ASSET_PIPELINE_HPP

#include <cstdint>
#include <map>

#include "asset/asset_defines.hpp"

namespace other {

  class AssetPipeline {
   public:
    static constexpr uint8_t kActionSize = 16;
    static constexpr uint8_t kCommandSize = 20;

    enum CommandType : uint16_t {
      FROM_COMMAND = 0x0000,
      TO_COMMAND = 0x0001,
    };

    struct Instruction {
      union {
        struct {
          uint64_t resource_hash;
          uint64_t action_hash;
        };
        uint8_t buffer[kActionSize];
      };
    };

    struct Command {
      /// union to allow use of command as pure bytes
      union {
        struct {
          CommandType type;
          Instruction instruction;
        };
        uint8_t buffer[kCommandSize];
      };
    };

    AssetPipeline() = default;
    ~AssetPipeline() = default;

    void AddCommand(const Command& command);
    void AddCommand(const CommandType type, const Instruction& instruction);

    uint64_t AddResource(const std::string_view name);
    bool HasResource(const std::string_view name) const;
    bool HasResource(const uint64_t name) const;

    const std::map<uint64_t, std::string>& GetResources() const;

    void Execute();

   private:
    std::vector<Command> commands;

    std::map<uint64_t, std::string> resource_names;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_PIPELINE_HPP