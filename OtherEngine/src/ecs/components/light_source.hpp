
/**
 * \file ecs/components/light_source.hpp
 **/
#ifndef OTHER_ENGINE_LIGHT__SOURCE_HPP
#define OTHER_ENGINE_LIGHT__SOURCE_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "rendering/direction_light.hpp"
#include "rendering/material.hpp"
#include "rendering/point_light.hpp"

namespace other {

  enum LightSourceType {
    DIRECTION_LIGHT_SRC = 0,
    POINT_LIGHT_SRC,

    NUM_LIGHT_SRCS,
    INVALID_LIGHT_SRC = NUM_LIGHT_SRCS,
  };

  struct LightSource : public Component {
    LightSourceType type = INVALID_LIGHT_SRC;
    union {
      DirectionLight direction_light;
      PointLight pointlight;
    };

    Opt<AssetHandle> debug_model = std::nullopt;

    // static Material debug_light_mat;

    ECS_COMPONENT(LightSource, LIGHTSOURCE_COMPONENT_INDEX);
  };

  class LightSourceSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(LightSource);
  };

}  // namespace other

ECHO_TYPE(
  type(other::LightSource, refl::attr::bases<other::Component>),
  field(type),
  field(direction_light),
  field(pointlight),
  field(debug_model)
);

#endif  // !OTHER_ENGINE_LIGHT_SOURCE_HPP
