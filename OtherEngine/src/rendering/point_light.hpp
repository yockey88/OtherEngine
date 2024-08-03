/**
 * \file rendering/point_light.hpp
 **/
#ifndef OTHER_ENGINE_POINT_LIGHT_HPP
#define OTHER_ENGINE_POINT_LIGHT_HPP

#include <glm/glm.hpp>

namespace other {

  struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant = 1.f;
    float linear = 0.09f;
    float quadratic = 0.032f;
  };
  
  struct DirectionLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant = 1.f;
    float linear = 0.09f;
    float quadratic = 0.032f;
  };

} // namespace other

#endif // !OTHER_ENGINE_POINT_LIGHT_HPP
