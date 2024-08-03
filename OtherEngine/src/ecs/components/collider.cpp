
/**
 * \file ecs/components/collider.cpp
 **/
#include "ecs/components/collider.hpp"

#include "core/logger.hpp"
#include "core/config_keys.hpp"

#include "ecs/entity.hpp"

namespace other {

  void ColliderSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& collider = entity->GetComponent<Collider>();
    
    SerializeComponentSection(stream , entity , "collider");
  }

  void ColliderSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a rigid-body into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kColliderValue });

    auto& collider = entity->GetComponent<Collider>();
  }

} // namespace other
