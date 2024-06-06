/**
 * \file ecs/systems/core_systems.cpp
 **/
#include "ecs/systems/core_systems.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"

namespace other {

  void OnConstructEntity(entt::registry& context , entt::entity entt) {
    Entity e(context , entt);
    auto& tag = e.AddComponent<Tag>();
    tag.id = 0;
    tag.name = "[ Blank Entity ]";
    
    /* auto& relationship = */ e.AddComponent<Relationship>();
    /* auto& transform = */ e.AddComponent<Transform>();
  }
  
  void OnDestroyEntity(entt::registry& context , entt::entity entt) {
  }

} // namespace other
