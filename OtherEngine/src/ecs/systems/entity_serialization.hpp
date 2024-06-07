/**
 * \file ecs/systems/entity_serialization.hpp
 **/
#ifndef OTHER_ENGINE_ENTITY_SERIALIZATION_HPP
#define OTHER_ENGINE_ENTITY_SERIALIZATION_HPP

#include "ecs/component_serializer.hpp"

namespace other {

  class EntitySerialization {
    public:
      EntitySerialization() = default;
      ~EntitySerialization() {}

      static Scope<ComponentSerializer> GetComponentSerializer(const std::string_view tag);
  };

} // namespace other

#endif // !OTHER_ENGINE_ENTITY_SERIALIZATION_HPP
