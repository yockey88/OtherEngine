/**
 * \file rendering/material.hpp
 **/
#ifndef OTHER_ENGINE_MATERIAL_HPP
#define OTHER_ENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

namespace other {

  struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
  };

} // namespace other

#endif // !OTHER_ENGINE_MATERIAL_HPP
