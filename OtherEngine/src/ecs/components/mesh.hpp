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

namespace other {

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

   private:
    AssetHandle GetMeshHandle(const std::string& path) const;
  };

  struct MeshSnapshotter : public ObjectSerializer<Mesh, 3> {
    MeshSnapshotter() {
      AddField<AssetHandle, 0>(&Mesh::handle);
      AddField<UUID, 1>(&Mesh::material);
      // AddField<std::vector<UUID>, 2>(&Mesh::bone_entity_ids);
      AddField<bool, 2>(&Mesh::visible);
    }
  };

  class StaticMeshSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(StaticMesh);
  };

  struct StaticMeshSnapshotter : public ObjectSerializer<StaticMesh, 6> {
    StaticMeshSnapshotter() {
      AddField<AssetHandle, 0>(&StaticMesh::handle);
      AddField<UUID, 1>(&StaticMesh::material);
      AddField<bool, 2>(&StaticMesh::visible);
      AddField<bool, 3>(&StaticMesh::is_primitive);
      AddField<uint32_t, 4>(&StaticMesh::primitive_id);
      AddField<uint32_t, 5>(&StaticMesh::primitive_selection);
    }
  };

}  // namespace other

ECHO_TYPE(
  type(other::Mesh),
  field(handle),
  field(material),
  field(bone_entity_ids),
  field(visible)
);

ECHO_TYPE(
  type(other::StaticMesh),
  field(handle),
  field(material),
  field(visible),
  field(is_primitive),
  field(primitive_id),
  field(primitive_selection)
);

#endif  // !OTHER_ENGINE_MESH_HPP
