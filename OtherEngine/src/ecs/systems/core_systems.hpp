/**
 * \file ecs/systems/core_systems.hpp
 **/
#ifndef OTHER_ENGINE_CORE_SYSTEMS_HPP
#define OTHER_ENGINE_CORE_SYSTEMS_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

#include "ecs/components/collider_2d.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/physics_component.hpp"
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

  void OnAddModel(entt::registry& context, entt::entity entt);
  void OnAddStaticModel(entt::registry& context, entt::entity entt);

}  // namespace other

#endif  // !OTHER_ENGINE_CORE_SYSTEMS_HPP
