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
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/rigid_body.hpp"
#include "ecs/components/collider.hpp"

#include "physics/2D/physics_world_2d.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  using SystemFn = void(*)(entt::registry&,entt::entity);

  // template <typename T>
  // struct OeSystemCallback {
  //   template <typename O>
  //   OeSysCall (entt::registry& reg , SysFunc fn , O* obj) {
  //   }
  // };

#define CORE_SYSTEM(name) \
  void name(entt::registry& context , entt::entity entt)

  CORE_SYSTEM(OnConstructEntity);
  CORE_SYSTEM(OnDestroyEntity);
  CORE_SYSTEM(OnCameraAddition);

  CORE_SYSTEM(OnAddModel);
  CORE_SYSTEM(OnAddStaticModel);
  
  void Initialize2DRigidBody(Ref<PhysicsWorld2D>& world , RigidBody2D& body , const Tag& tag , const Transform& transform);
  void Initialize2DCollider(Ref<PhysicsWorld2D>& world , RigidBody2D& body , Collider2D& collider , const Transform& transform);

  CORE_SYSTEM(OnRigidBody2DUpdate);
  CORE_SYSTEM(OnCollider2DUpdate);
  
  void InitializeRigidBody(Ref<PhysicsWorld>& world , RigidBody& body , const Tag& tag , const Transform& transform);
  void InitializeCollider(Ref<PhysicsWorld>& world , RigidBody& body , Collider& collider , const Transform& transform);

  CORE_SYSTEM(OnRigidBodyUpdate);
  CORE_SYSTEM(OnColliderUpdate);

} // namespace other

#endif // !OTHER_ENGINE_CORE_SYSTEMS_HPP
