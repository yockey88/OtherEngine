/**
 * \file rendering/point_light.hpp
 **/
#ifndef OTHER_ENGINE_POINT_LIGHT_HPP
#define OTHER_ENGINE_POINT_LIGHT_HPP

#include <glm/glm.hpp>

namespace other {

  struct PointLight {
    glm::vec4 position;
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float constant = 1.f;
    float linear = 0.09f;
    float quadratic = 0.032f;
    float padding;
  };

  static_assert(sizeof(PointLight) == 5 * 4 * sizeof(float) , "PointLight size invalid for GPU layout!");
  static_assert(sizeof(PointLight) % 16 == 0 , "PointLight size not a multiple of 16!");
  
} // namespace other

#endif // !OTHER_ENGINE_POINT_LIGHT_HPP
