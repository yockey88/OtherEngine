/**
 * \file math/bounding_box.hpp
 **/
#ifndef OTHER_ENGINE_BOUNDING_BOX_HPP
#define OTHER_ENGINE_BOUNDING_BOX_HPP

#include <array>
#include <string_view>

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <spdlog/fmt/bundled/format.h>

#include "core/defines.hpp"
#include "math/ray.hpp"
#include "math/vecmath.hpp"

namespace other {

  constexpr inline static size_t kNumCubeCorners = 8;

  struct BBox {
    glm::vec3 min{0.f, 0.f, 0.f};
    glm::vec3 max{0.f, 0.f, 0.f};
    glm::vec3 extent{0.f, 0.f, 0.f};

    std::array<glm::vec3, kNumCubeCorners> corners = {};

    BBox(const glm::vec3& min, const glm::vec3& max)
        : min(min), max(max), extent(vec3_sub(max, min)) {
      CalculateCorners();
    }
    BBox(const glm::vec3& point)
        : BBox(point, point) {}
    BBox()
        : BBox(glm::vec3{0.f}) {}

    bool Contains(const glm::vec3& point) const;
    bool OnBoundary(const glm::vec3& point) const;

    Intersection Intersect(const Ray& ray) const;

    void ExpandToInclude(const glm::vec3& point);
    void ExpandToFill(const BBox& other);

    glm::vec3 Center() const;
    bool Intersects(/* ray , near , far */) const { return false; }

    float MaxDimension() const;
    float SurfaceArea() const;

    const std::array<glm::vec3, 8>& Corners() const;

    static BBox Union(const BBox& a, const BBox& b) {
      BBox result;
      result.min = {
        glm::min(a.min.x, b.min.x),
        glm::min(a.min.y, b.min.y),
        glm::min(a.min.z, b.min.z)};
      result.max = {
        glm::max(a.max.x, b.max.x),
        glm::max(a.max.y, b.max.y),
        glm::max(a.max.z, b.max.z)};
      result.extent = vec3_sub(result.max, result.min);
      return result;
    }

   private:
    void CalculateCorners();
  };

}  // namespace other

template <>
struct fmt::formatter<other::BBox> : fmt::formatter<std::string_view> {
  auto format(const other::BBox& bbox, fmt::format_context& ctx) {
    std::stringstream ss;
    ss << other::fmtstr("BBOX(@min{} , @max{} , @extent{})", bbox.min, bbox.max, bbox.extent);
    return fmt::formatter<std::string_view>::format(ss.str(), ctx);
  }
};

#endif  // !OTHER_ENGINE_BOUNDING_BOX_HPP