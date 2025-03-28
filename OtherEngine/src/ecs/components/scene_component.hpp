/**
 * \file ecs/components/scene_component.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_SCENE_COMPONENT_HPP
#define OTHER_ENGINE_ENTITY_SCENE_COMPONENT_HPP

#include <cstdint>
#include <set>

#include "ecs/component.hpp"

namespace other {

  struct SceneComponent : public Component {
    std::set<int32_t> entity_components;
    INVISIBLE_ECS_COMPONENT(SceneComponent, -2);
  };

}  // namespace other

ECHO_TYPE(
  type(other::SceneComponent, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_ENTITY_SCENE_COMPONENT_HPP
