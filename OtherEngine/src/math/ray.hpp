/**
 * \file math/ray.hpp
 **/
#ifndef OTHER_ENGINE_RAY_HPP
#define OTHER_ENGINE_RAY_HPP

#include <glm/glm.hpp>

#include "math/vecmath.hpp"

namespace other {

  struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 At(float t) {
      return vec3_sum(origin , vec3_product(t , direction));
    }
  };

  struct Intersection {
    /// interaction information*
    Opt<glm::vec3> point = std::nullopt;
  };

} // namespace other

#endif // !OTHER_ENGINE_RAY_HPP