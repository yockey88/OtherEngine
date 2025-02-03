/**
 * \file ecs/components/mesh.hpp
 **/
#ifndef OTHER_ENGINE_MESH_HPP
#define OTHER_ENGINE_MESH_HPP

#include <reflection/echo_defines.hpp>
#include <reflection/reflected_object.hpp>

#include "asset/asset_defines.hpp"

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "rendering/material.hpp"

#include "serialization/scene_file_format_defines.hpp"

namespace other {

  struct Mesh;
  struct StaticMesh;

  struct MeshSnapshotter : public ObjectSerializer<Mesh, 3> {
    MeshSnapshotter();

    static size_t Stride() {
      return kMaxStringLength + sizeof(UUID) + sizeof(bool);
    }
  };

  struct StaticMeshSnapshotter : public ObjectSerializer<StaticMesh, 5> {
    StaticMeshSnapshotter();

    static size_t Stride() {
      return sizeof(UUID) + sizeof(bool) + sizeof(bool) + sizeof(uint32_t) * 2;
    }
  };

  struct Mesh : public Component {
    AssetHandle handle;
    UUID material;
    std::vector<UUID> bone_entity_ids;
    bool visible = true;

    ECS_COMPONENT(Mesh, MESH_COMPONENT_INDEX);
  };

  /// primitive mesh types
  constexpr static uint32_t kEmptyIdx = 0;
  constexpr static uint32_t kTriangleIdx = 1;
  constexpr static uint32_t kRectIdx = 2;
  constexpr static uint32_t kCubeIdx = 3;
  constexpr static uint32_t kSphereIdx = 4;
  constexpr static uint32_t kCapsuleIdx = 5;

  struct StaticMesh : public Component {
    AssetHandle handle;
    UUID material;
    bool visible = true;

    bool is_primitive = false;
    uint32_t primitive_id = 0;
    uint32_t primitive_selection = 0;

    ECS_COMPONENT(StaticMesh, STATICMESH_COMPONENT_INDEX);
  };

  class MeshSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Mesh);
  };

  class StaticMeshSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(StaticMesh);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Mesh),
  field(handle, echo::serializable_field()),
  field(material, echo::serializable_field()),
  field(bone_entity_ids),
  field(visible, echo::serializable_field())
);

ECHO_TYPE(
  type(other::StaticMesh),
  field(handle, echo::serializable_field()),
  field(material, echo::serializable_field()),
  field(visible, echo::serializable_field()),
  field(is_primitive, echo::serializable_field()),
  field(primitive_id, echo::serializable_field()),
  field(primitive_selection)
);

#endif  // !OTHER_ENGINE_MESH_HPP
