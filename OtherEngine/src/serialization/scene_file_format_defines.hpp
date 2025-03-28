/**
 * \file serialization/scene_file_format_defines.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_FILE_FORMAT_DEFINES_HPP
#define OTHER_ENGINE_SCENE_FILE_FORMAT_DEFINES_HPP

#include "ecs/component.hpp"

namespace other {

  /// maximum path length on windows and linux is 260
  constexpr static size_t kMaxStringLength = 260;
  constexpr static size_t kMaxNameLength = kMaxStringLength;
  /// this is per scene packet, not per scene
  constexpr static size_t kMaxGroups = 16;

  /// how to extend this to support more components?
  constexpr static size_t kMaxEntities = 1024;

#pragma pack(push, 1)
  enum GroupId : uint16_t {
    EMPTY_GROUP = 0x00,
    ENTITY_GROUP = 0x01,
  };

  struct GroupIndex {
    GroupId group_id = EMPTY_GROUP;
    uint64_t offset = 0;
  };

  struct LayoutTable {
    uint16_t num_groups = 0;
    GroupIndex groups[kMaxGroups];
  };

  struct SceneMeta {
    uint16_t name_len = 0;
    char name[kMaxNameLength] = { 0 };
  };

  struct PhysicsTable {
    glm::vec2 gravity_2d = { 0.f, 0.f };
  };

  struct SceneHeader {
    SceneMeta meta;
    LayoutTable layout;
    /// add physics table here
  };

  struct EntityGroupHeader {
    uint64_t num_entities = 0;
  };

  struct ComponentBit {
    uint8_t flag : 1;
  };

  struct EntityDescriptor {
    uint64_t entity_id = 0;
    uint64_t component_table_offset = 0;
    uint64_t component_list_offset = 0;
    uint64_t component_list_len = 0;
    uint16_t name_len = 0;
    char name[kMaxNameLength] = { 0 };
  };

  struct ComponentTable {
    uint8_t num_components = 0;
    ComponentBit components[NUM_COMPONENTS];
  };
#pragma pack(pop)

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_FILE_FORMAT_DEFINES_HPP