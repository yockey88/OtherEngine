/**
 * \file scene/light_environment.hpp
 **/
#ifndef OTHER_ENGINE_LIGHT_ENVIRONMENT_HPP
#define OTHER_ENGINE_LIGHT_ENVIRONMENT_HPP

#include <vector>

#include "core/ref_counted.hpp"

#include "rendering/direction_light.hpp"
#include "rendering/point_light.hpp"

namespace other {

  class LightEnvironment : public RefCounted {
   public:
    std::vector<DirectionLight> direction_lights;
    std::vector<PointLight> point_lights;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_LIGHT_ENVIRONMENT_HPP
