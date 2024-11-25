/**
 * \file math/ray.cpp
 **/
#include "math/ray.hpp"

#include "math/vecmath.hpp"

#include "ecs/entity.hpp"

namespace other {

  void TraceResult::SetFaceNormal(const Ray& r, const glm::vec3& outward_normal) {
    front_face = glm::dot(r.direction, outward_normal) < 0;
    normal = front_face ?
      outward_normal :
      -outward_normal;
  }

  glm::vec3 Ray::At(float t) const {
    glm::vec3 scaled_dir = vec3_product(t, direction);
    return vec3_sum(origin, scaled_dir);
  }

}  // namespace other
