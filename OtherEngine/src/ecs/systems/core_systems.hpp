/**
 * \file ecs/systems/core_systems.hpp
 **/
#ifndef OTHER_ENGINE_CORE_SYSTEMS_HPP
#define OTHER_ENGINE_CORE_SYSTEMS_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

#include "ecs/components/collider.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/rigid_body.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"

#include "physics/2D/physics_world_2d.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  using SystemFn = void (*)(entt::registry&, entt::entity);

  void OnConstructEntity(entt::registry& context, entt::entity entt);
  void OnDestroyEntity(entt::registry& context, entt::entity entt);

  void OnCameraAddition(entt::registry& context, entt::entity entt);

  void OnAddScript(entt::registry& context, entt::entity entt);

  void OnAddModel(entt::registry& context, entt::entity entt);
  void OnAddStaticModel(entt::registry& context, entt::entity entt);

  void OnAddRigidBody2D(entt::registry& context, entt::entity entt);
  void OnRigidBody2DUpdate(entt::registry& context, entt::entity entt);

  void OnAddRigidBody(entt::registry& context, entt::entity entt);
  void OnRigidBodyUpdate(entt::registry& context, entt::entity entt);

  void OnAddCollider2D(entt::registry& context, entt::entity entt);
  void OnCollider2DUpdate(entt::registry& context, entt::entity entt);

  void OnAddCollider(entt::registry& context, entt::entity entt);
  void OnColliderUpdate(entt::registry& context, entt::entity entt);

  void Initialize2DRigidBody(Ref<PhysicsWorld2D>& world, RigidBody2D& body, const Tag& tag, const Transform& transform);
  void Initialize2DCollider(Ref<PhysicsWorld2D>& world, RigidBody2D& body, Collider2D& collider, const Transform& transform);

}  // namespace other

#endif  // !OTHER_ENGINE_CORE_SYSTEMS_HPP
