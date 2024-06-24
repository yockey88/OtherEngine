/**
 * \file ecs/systems/core_systems.hpp
 **/
#ifndef OTHER_ENGINE_CORE_SYSTEMS_HPP
#define OTHER_ENGINE_CORE_SYSTEMS_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/rigid_body_2d.hpp"

#include "physics/2D/physics_world_2d.hpp"

namespace other {

#define CORE_SYSTEM(name) \
  void name(entt::registry& context , entt::entity entt)

  CORE_SYSTEM(OnConstructEntity);
  CORE_SYSTEM(OnDestroyEntity);
  CORE_SYSTEM(OnCameraAddition);
  
  void Initialize2DRigidBody(Ref<PhysicsWorld2D>& world , RigidBody2D& body , const Tag& tag , const Transform& transform);

  CORE_SYSTEM(OnRigidBody2DUpdate);

} // namespace other

#endif // !OTHER_ENGINE_CORE_SYSTEMS_HPP
