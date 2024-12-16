/**
 * \file rendering/material.hpp
 **/
#ifndef OTHER_ENGINE_MATERIAL_HPP
#define OTHER_ENGINE_MATERIAL_HPP

#include <cstdint>

#include <glm/glm.hpp>

#include "core/ref.hpp"

#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"

namespace other {

  struct Material {
    uint32_t albedo_tex_idx = 0;
    uint32_t normal_tex_idx = 0;
    uint32_t roughness_tex_idx = 0;
    uint32_t padding = 0;
  };

  static_assert(sizeof(Material) == (4 * sizeof(uint32_t)), "Material size is invalid for GPU layout!");
  // // static_assert(sizeof(Material) == 4 * sizeof(uint32_t), "Material size is invalid for GPU layout!");
  static_assert(sizeof(Material) % 16 == 0, "Material size not a multiple of 16!");

}  // namespace other

#endif  // !OTHER_ENGINE_MATERIAL_HPP
