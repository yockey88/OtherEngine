/**
 * \file ecs/component_serializer.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_SERIALIZER_HPP
#define OTHER_ENGINE_COMPONENT_SERIALIZER_HPP

namespace other {

  class Entity;

  class ComponentSerializer {
    public:
      virtual ~ComponentSerializer() {}
  };

} // namespace other 

#endif // !OTHER_ENGINE_COMPONENT_SERIALIZER_HPP
