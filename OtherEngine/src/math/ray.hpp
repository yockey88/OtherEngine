/**
 * \file math/ray.hpp
 **/
#ifndef OTHER_ENGINE_RAY_HPP
#define OTHER_ENGINE_RAY_HPP

#include <glm/glm.hpp>
#include <reflection/echo_defines.hpp>
#include <spdlog/fmt/fmt.h>

#include "core/formatters.hpp"

namespace other {

  class Entity;

  struct Ray {
    glm::vec3 origin;
    glm::vec3 direction;

    glm::vec3 At(float t) const;
  };

  struct TraceResult {
    glm::vec3 point;
    glm::vec3 normal;
    bool front_face;

    float t;  // t := parameterizes how far along ray the hit was

    Entity* hit_entity = nullptr;

    // Material mat;
    // float u;
    // float v;

    void SetFaceNormal(const Ray& r, const glm::vec3& outward_normal);
  };

}  // namespace other

template <>
struct fmt::formatter<other::Ray> : public fmt::formatter<std::string_view> {
  auto format(const other::Ray& ray, fmt::format_context& ctx) {
    std::string res = other::fmtstr("Ray(origin = {}, direction = {})", ray.origin, glm::normalize(ray.direction));
    return fmt::formatter<std::string_view>::format(res, ctx);
  }
};

ECHO_TYPE(
  type(other::Ray),
  field(origin),
  field(direction)
)

#endif  // !OTHER_ENGINE_RAY_HPP
