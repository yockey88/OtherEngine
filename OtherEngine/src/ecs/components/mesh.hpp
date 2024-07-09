/**
 * \file ecs/components/mesh.hpp
 **/
#ifndef OTHER_ENGINE_MESH_HPP
#define OTHER_ENGINE_MESH_HPP

#include "asset/asset_types.hpp"
#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Mesh : public Component {
    AssetHandle handle; 
    // Ref<MaterialTable> mat_table = nullptr;
    std::vector<UUID> bone_entity_ids;
    bool visible = true;

    ECS_COMPONENT(Mesh , kMeshIndex);
  }; 

  constexpr static uint32_t kEmptyIdx = 0;
  constexpr static uint32_t kTriangleIdx = 1;
  constexpr static uint32_t kRectIdx = 2;
  constexpr static uint32_t kCubeIdx = 3;
  constexpr static uint32_t kSphereIdx = 4;
  constexpr static uint32_t kCapsuleIdx = 5;
  
  struct StaticMesh : public Component {
    AssetHandle handle; 
    // Ref<MaterialTable> mat_table = nullptr;
    bool visible = true;

    bool is_primitive = false;
    uint32_t primitive_id = 0; /// 1 = cube , 2 = sphere , 3 = capsule
    uint32_t primitive_selection = 0;

    ECS_COMPONENT(StaticMesh , kStaticMeshIndex);
  }; 

  class MeshSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Mesh);
  };
  
  class StaticMeshSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(StaticMesh);
  };

} // namespace other

#endif // !OTHER_ENGINE_MESH_HPP
