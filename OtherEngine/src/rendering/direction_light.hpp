
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

  static_assert(sizeof(DirectionLight) == 4 * 4 * sizeof(float) , "DirectionLight has incorrect size for GPU layout!");
  static_assert(sizeof(DirectionLight) % 16 == 0 , "DirectionLight size not a multiple of 16!");

} // namespace other

#endif // !OTHER_ENGINE_DIRECTION_LIGHT_HPP
