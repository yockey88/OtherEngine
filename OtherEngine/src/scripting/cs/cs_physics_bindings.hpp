/**
 * \file scripting/cs/cs_physics_bindings.hpp
 **/
#ifndef OTHER_ENGINE_CS_PHYSICS_BINDINGS_HPP
#define OTHER_ENGINE_CS_PHYSICS_BINDINGS_HPP

#include <cstdint>

#include <glm/glm.hpp>

#include "math/ray.hpp"

namespace other {
  namespace cs_script_bindings {

    bool NativeRaycast(Ray* ray, float* ray_length, uint64_t* hit_entity, glm::vec3* hit_normal, glm::vec3* hit_point, float* hit_distance);  // ,  uint32_t layer_mask, uint32_t group_mask, uint32_t mask);

  }  // namespace cs_script_bindings
}  // namespace other

#endif  // !OTHER_ENGINE_CS_PHYSICS_BINDINGS_HPP