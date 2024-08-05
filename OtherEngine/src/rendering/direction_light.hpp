
/**
 * \file rendering/direction_light.hpp
 **/
#ifndef OTHER_ENGINE_DIRECTION_LIGHT_HPP
#define OTHER_ENGINE_DIRECTION_LIGHT_HPP

#include <glm/glm.hpp>

namespace other {

  struct DirectionLight {
    glm::vec4 direction;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
  };

} // namespace other

#endif // !OTHER_ENGINE_DIRECTION_LIGHT_HPP
