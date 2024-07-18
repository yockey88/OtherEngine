/**
 * \file math/aabb.hpp
 **/
#ifndef OTHER_ENGINE_AABB_HPP
#define OTHER_ENGINE_AABB_HPP

#include <glm/glm.hpp>

namespace other {

  struct AABB {
    glm::vec3 min  = glm::vec3(1.f);
    glm::vec3 max = glm::vec3(1.f);
  };

} // namespace other

#endif // !OTHER_ENGINE_AABB_HPP
