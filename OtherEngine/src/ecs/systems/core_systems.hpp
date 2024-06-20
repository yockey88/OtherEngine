/**
 * \file ecs/systems/core_systems.hpp
 **/
#ifndef OTHER_ENGINE_CORE_SYSTEMS_HPP
#define OTHER_ENGINE_CORE_SYSTEMS_HPP

#include <entt/entity/fwd.hpp>
#include <entt/entt.hpp>

namespace other {

#define CORE_SYSTEM(name) \
  void name(entt::registry& context , entt::entity entt)

  CORE_SYSTEM(OnConstructEntity);
  CORE_SYSTEM(OnDestroyEntity);
  CORE_SYSTEM(OnCameraAddition);

} // namespace other

#endif // !OTHER_ENGINE_CORE_SYSTEMS_HPP
