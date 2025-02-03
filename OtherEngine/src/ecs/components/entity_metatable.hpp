/**
 * \file ecs/components/entity_metatable.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_METATABLE_HPP
#define OTHER_ENGINE_ENTITY_METATABLE_HPP

#include "ecs/component.hpp"

namespace other {

  struct EntityMetatable : public Component {
    Component* components[kNumComponents] = { nullptr };
    INVISIBLE_ECS_COMPONENT(EntityMetatable, -3);
  };

}  // namespace other

ECHO_TYPE(
  type(other::EntityMetatable, refl::attr::bases<other::Component>)
)

#endif  // !OTHER_ENGINE_ENTITY_METATABLE_HPP