/**
 * \file scene/light_environment.hpp
 **/
#ifndef OTHER_ENGINE_LIGHT_ENVIRONMENT_HPP
#define OTHER_ENGINE_LIGHT_ENVIRONMENT_HPP

#include <vector>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"

#include "rendering/direction_light.hpp"
#include "rendering/point_light.hpp"


namespace other {

  class LightEnvironment : public RefCounted {
   public:
    Opt<DirectionLight> direction_light;
    std::vector<PointLight> point_lights;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_LIGHT_ENVIRONMENT_HPP
