
/**
 * \file ecs/components/rigid_body.cpp
 **/
#include "ecs/components/rigid_body.hpp"

#include "core/logger.hpp"
#include "core/config_keys.hpp"

#include "ecs/entity.hpp"

namespace other {

  void RigidBodySerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    auto& rigid_body = entity->GetComponent<RigidBody>();
    
    SerializeComponentSection(stream , entity , "rigid-body");
  }

  void RigidBodySerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize a rigid-body into null entity or scene!");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kRigidBodyValue });

    auto& rigid_body = entity->GetComponent<RigidBody>();
  }

} // namespace other
