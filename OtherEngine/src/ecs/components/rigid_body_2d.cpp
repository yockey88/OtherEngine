/**
 * \file ecs/components/rigid_body_2d.cpp
 **/
#include "ecs/components/rigid_body_2d.hpp"

namespace other {

  void RigidBody2DSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {}

  void RigidBody2DSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {}

} // namespace other
