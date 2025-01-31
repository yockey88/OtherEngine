/**
 * \file ecs/systems/component_database.cpp
 **/
#include "ecs/systems/component_database.hpp"

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

}  // namespace other