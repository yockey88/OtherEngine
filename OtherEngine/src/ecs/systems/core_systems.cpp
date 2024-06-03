/**
 * \file ecs/systems/core_systems.cpp
 **/
#include "ecs/systems/core_systems.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"

namespace other {

  void OnConstructEntity(entt::registry& context , entt::entity entt) {
    /// all entities must have a tag and a transform
    Entity e(context , entt);
    e.AddComponent<Tag>();
    e.AddComponent<Transform>();
  }
  
  void OnDestroyEntity(entt::registry& context , entt::entity entt) {
    // Entity e(context , entt);
  }

} // namespace other
