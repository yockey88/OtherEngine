/**
 * \file environment/command.hpp
 **/
#ifndef OTHER_ENGINE_COMMAND_HPP
#define OTHER_ENGINE_COMMAND_HPP

#include <cstdint>

#include <spdlog/fmt/fmt.h>

#include "core/defines.hpp"
#include "environment/env_defines.hpp"

namespace other {

  /**
   * Command Structure:
   *
   * 64 bits in total
   *  command option
   *  - 8 for priority
   *  - 8 for category (project,scene,...)
   *  - 8 for command
   *  - 8 for number of arguments
   *  - 32 bits as padding ???
   *
   *  argument address option
   **/

  enum CommandPriority : uint8_t {
    IGNORE_COMMAND = 0x00,
    DEFAULT_COMMAND_PRIORITY = 0x01,
    LOW_COMMAND_PRIORITY = 0x02,
    MEDIUM_COMMAND_PRIORITY = 0x03,
    HIGH_COMMAND_PRIORITY = 0x04,
    URGENT_COMMAND = 0x05,

    NUM_COMMAND_PRIORITIES,
    INVALID_COMMAND_PRIORITY = NUM_COMMAND_PRIORITIES,
  };

  enum CommandCategory : uint8_t {
    EMPTY_COMMAND = 0x00,
    PROJECT_COMMAND = 0x01,
    SCENE_COMMAND = 0x02,
    ENVIRONMENT_COMMAND = 0x03,

    NUM_COMMAND_CATEGORIES,
    INVALID_COMMAND_CATEGORY = NUM_COMMAND_CATEGORIES,
  };

  enum CommandType : uint8_t {
    NO_OP_COMMAND = 0x00,
    CREATE_COMMAND = 0x01,
    DELETE_COMMAND = 0x02,
    LOAD_COMMAND = 0x03,
    UNLOAD_COMMAND = 0x04,
    SAVE_COMMAND = 0x05,
    CLEAR_COMMAND = 0x06,
    EXIT_COMMAND /* = 0x0? */,

    NUM_COMMAND_TYPES,
    INVALID_COMMAND_TYPE = NUM_COMMAND_TYPES,
  };

#pragma pack(push, 1)
  struct Command {
    union {
      struct {
        CommandPriority priority;
        CommandCategory category;
        CommandType command;
        uint8_t num_args;
        uint32_t padding;
      };

      address_t argument_address;
    };
  };
  static_assert(sizeof(Command) == sizeof(uint64_t), "Command is the wrong size!");
#pragma pack(pop)

  struct CommandMap {
    using PriorityPair = std::pair<uint64_t, CommandPriority>;
    using CategoryPair = std::pair<uint64_t, CommandCategory>;
    using TypePair = std::pair<uint64_t, CommandType>;

    constexpr static size_t kNumPriorities = NUM_COMMAND_PRIORITIES;
    constexpr static size_t kNumCategories = NUM_COMMAND_CATEGORIES;
    constexpr static size_t kNumCommands = NUM_COMMAND_TYPES;

    constexpr static std::array<PriorityPair, kNumPriorities> kCommandPriority = {
      PriorityPair{ FNV("ignore"), IGNORE_COMMAND },
      PriorityPair{ FNV("defaultp"), DEFAULT_COMMAND_PRIORITY },
      PriorityPair{ FNV("lowp"), LOW_COMMAND_PRIORITY },
      PriorityPair{ FNV("mediump"), MEDIUM_COMMAND_PRIORITY },
      PriorityPair{ FNV("highp"), HIGH_COMMAND_PRIORITY },
      PriorityPair{ FNV("urgentp"), URGENT_COMMAND },
    };

    constexpr static std::array<CategoryPair, kNumCategories> kCommandCategory = {
      CategoryPair{ FNV("empty-command"), EMPTY_COMMAND },
      CategoryPair{ FNV("project"), PROJECT_COMMAND },
      CategoryPair{ FNV("scene"), SCENE_COMMAND },
    };

    constexpr static std::array<TypePair, kNumCommands> kCommandType = {
      TypePair{ FNV("no-op"), NO_OP_COMMAND },
      TypePair{ FNV("clear"), CLEAR_COMMAND },
      TypePair{ FNV("exit"), EXIT_COMMAND },
      TypePair{ FNV("create"), CREATE_COMMAND },
      TypePair{ FNV("delete"), DELETE_COMMAND },
      TypePair{ FNV("load"), LOAD_COMMAND },
      TypePair{ FNV("unload"), UNLOAD_COMMAND },
      TypePair{ FNV("save"), SAVE_COMMAND },
    };
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COMMAND_HPP
