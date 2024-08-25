
/**
 * \file rendering/direction_light.hpp
 **/
#ifndef OTHER_ENGINE_DIRECTION_LIGHT_HPP
#define OTHER_ENGINE_DIRECTION_LIGHT_HPP

#include <glm/glm.hpp>

namespace other {

  struct DirectionLight {
    glm::vec4 direction{ 0.f , -1.f , 0.f , 1.f };
    glm::vec4 color{ 1.f , 1.f , 1.f , 1.f };
  };

  static_assert(sizeof(DirectionLight) == 2 * 4 * sizeof(float) , "DirectionLight has incorrect size for GPU layout!");
  static_assert(sizeof(DirectionLight) % 16 == 0 , "DirectionLight size not a multiple of 16!");

} // namespace other

#endif // !OTHER_ENGINE_DIRECTION_LIGHT_HPP
