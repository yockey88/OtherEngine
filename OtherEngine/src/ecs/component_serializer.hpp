/**
 * \file ecs/component_serializer.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_SERIALIZER_HPP
#define OTHER_ENGINE_COMPONENT_SERIALIZER_HPP

#include "core/config.hpp"

namespace other {

  class Entity;

  class ComponentSerializer {
    public:
      virtual ~ComponentSerializer() {}

      virtual void Serialize(std::ostream& stream , Entity* owner) const = 0;
      virtual void Deserialize(Entity* entity , const ConfigTable& scn_table) const = 0;
  };

} // namespace other 

#endif // !OTHER_ENGINE_COMPONENT_SERIALIZER_HPP
