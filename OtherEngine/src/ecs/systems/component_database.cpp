/**
 * \file ecs/systems/component_database.cpp
 **/
#include "ecs/systems/component_database.hpp"

#include "ecs/component.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/physics_component.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/terrain.hpp"
#include "ecs/components/transform.hpp"

namespace other {

  std::array<ComponentDataBase::ComponentData, kNumComponents> ComponentDataBase::kComponentData = {
    ComponentDataBase::ComponentData{ &kComponentTags[TAG_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[TRANSFORM_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[RELATIONSHIP_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[MESH_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[STATICMESH_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[SCRIPT_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[CAMERA_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[RIGIDBODY2D_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[COLLIDER2D_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[RIGIDBODY_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[COLLIDER_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[PHYSICS_OBJECT_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[LIGHTSOURCE_COMPONENT_INDEX] },
    ComponentDataBase::ComponentData{ &kComponentTags[TERRAIN_COMPONENT_INDEX] }
  };

  using SizeQuery = std::function<size_t()>;
  static std::array<SizeQuery, kNumComponents> kComponentSizeQueries = {
    []() -> size_t { return 0; },  // tag component
    []() -> size_t { return TransformSnapshotter::Stride(); },
    []() -> size_t { return RelationshipSnapshotter::Stride(); },
    []() -> size_t { return MeshSnapshotter::Stride(); },
    []() -> size_t { return StaticMeshSnapshotter::Stride(); },
    []() -> size_t { return ScriptSnapshotter::Stride(); },
    []() -> size_t { return CameraSnapshotter::Stride(); },
    []() -> size_t { return RigidBody2DSnapshotter::Stride(); },
    []() -> size_t { return Collider2DSnapshotter::Stride(); },
    []() -> size_t { return RigidBodySnapshotter::Stride(); },
    []() -> size_t { return ColliderSnapshotter::Stride(); },
    []() -> size_t { return PhysicsObjectSnapshotter::Stride(); },
    []() -> size_t { return LightSourceSnapshotter::Stride(); },
    []() -> size_t { return TerrainSnapshotter::Stride(); },
  };

  std::string ComponentDataBase::GetComponentTagUc(size_t idx) {
    auto itr = std::find_if(kComponentTags.begin(), kComponentTags.end(), [&idx](const auto& tag_pair) -> bool {
      return idx == tag_pair.idx;
    });

    if (itr == kComponentTags.end()) {
      return "";
    }

    return std::string{ (*itr).name };
  }

  std::string ComponentDataBase::GetComponentTagLc(size_t idx) {
    auto itr = std::find_if(kComponentTags.begin(), kComponentTags.end(), [&idx](const auto& tag_pair) -> bool {
      return idx == tag_pair.idx;
    });

    if (itr == kComponentTags.end()) {
      return "";
    }

    std::string lc_tag;
    std::transform((*itr).name.begin(), (*itr).name.end(), std::back_inserter(lc_tag), ::tolower);

    return lc_tag;
  }

  int32_t ComponentDataBase::GetComponentIdxFromTag(const std::string_view tag) {
    std::string uc_tag;
    std::transform(tag.begin(), tag.end(), std::back_inserter(uc_tag), ::toupper);

    UUID hash = FNV(uc_tag);
    auto itr = std::find_if(kComponentTags.begin(), kComponentTags.end(), [&hash](const auto& tag_pair) -> bool {
      return hash.Get() == FNV(tag_pair.name);
    });

    if (itr == kComponentTags.end()) {
      return -1;
    }

    return (*itr).idx;
  }

  size_t ComponentDataBase::GetComponentStride(size_t idx) {
    OE_ASSERT(idx < kNumComponents, "Invalid component index");
    return kComponentSizeQueries[idx]();
  }

}  // namespace other