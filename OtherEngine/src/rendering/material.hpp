/**
 * \file rendering/material.hpp
 **/
#ifndef OTHER_ENGINE_MATERIAL_HPP
#define OTHER_ENGINE_MATERIAL_HPP

#include <glm/glm.hpp>

namespace other {

  struct Material {
    glm::vec4 ambient{ 0.5f , 0.5f , 0.5f , 1.f };
    glm::vec4 diffuse{ 0.5f , 0.5f , 0.5f , 1.f };
    glm::vec4 specular{ 0.5f , 0.5f , 0.5f , 1.f };
    float shininess = 32.f;
    float padding[3];
  };

  static_assert(sizeof(Material) == 4 * (4 * sizeof(float)) , "Material size is invalid for GPU layout!");
  static_assert(sizeof(Material) % 16 == 0 , "Material size not a multiple of 16!");

} // namespace other

#endif // !OTHER_ENGINE_MATERIAL_HPP
