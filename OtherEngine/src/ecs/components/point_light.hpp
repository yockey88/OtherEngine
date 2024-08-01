
/**
 * \file ecs/components/point_light.hpp
 **/
#ifndef OTHER_ENGINE_POINT_LIGHT_HPP
#define OTHER_ENGINE_POINT_LIGHT_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct PointLight : public Component {
    
  };

  class PointLightSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(PointLight);
  };

} // namespace other

#endif // !OTHER_ENGINE_POINT_LIGHT_HPP
