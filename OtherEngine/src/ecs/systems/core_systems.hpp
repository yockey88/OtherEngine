/**
 * \file ecs/systems/core_systems.hpp
 **/
#ifndef OTHER_ENGINE_CORE_SYSTEMS_HPP
#define OTHER_ENGINE_CORE_SYSTEMS_HPP

#include <entt/entt.hpp>

namespace other {

  void OnConstructEntity(entt::registry& context , entt::entity entt); 
  void OnDestroyEntity(entt::registry& context , entt::entity entt);

} // namespace other

#endif // !OTHER_ENGINE_CORE_SYSTEMS_HPP
