/**
 * \file rendering/material.hpp
 **/
#ifndef OTHER_ENGINE_MATERIAL_HPP
#define OTHER_ENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

namespace other {

  struct Material {
    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    float shininess;
    float padding[3];
  };

  static_assert(sizeof(Material) == 4 * (4 * sizeof(float)) , "Material size is invalid for GPU layout!");

} // namespace other

#endif // !OTHER_ENGINE_MATERIAL_HPP
