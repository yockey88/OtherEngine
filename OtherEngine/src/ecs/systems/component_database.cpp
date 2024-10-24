/**
 * \file ecs/systems/component_database.cpp
 **/
#include "ecs/systems/component_database.hpp"

namespace other {

  std::array<ComponentDataBase::ComponentData, kNumComponents> ComponentDataBase::kComponentData = {
    ComponentDataBase::ComponentData{ &kComponentTags[kTagIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kTransformIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kRelationshipIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kMeshIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kStaticMeshIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kScriptIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kCameraIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kRigidBody2DIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kCollider2DIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kRigidBodyIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kColliderIndex] },
    ComponentDataBase::ComponentData{ &kComponentTags[kLightSourceIndex] },
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