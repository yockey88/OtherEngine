/**
 * \file scene/environment.hpp
 **/
#ifndef OTHER_ENGINE_ENVIRONMENT_HPP
#define OTHER_ENGINE_ENVIRONMENT_HPP

#include <vector>

#include "core/ref_counted.hpp"

#include "rendering/direction_light.hpp"
#include "rendering/point_light.hpp"

namespace other {

  class Environment : public RefCounted {
    public:
      std::vector<DirectionLight> direction_lights;
      std::vector<PointLight> point_lights;
  };

} // namespace other

#endif // !OTHER_ENGINE_ENVIRONMENT_HPP
