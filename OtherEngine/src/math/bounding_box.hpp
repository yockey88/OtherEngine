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
#include "math/interval.hpp"
#include "math/ray.hpp"

namespace other {

  constexpr inline static size_t kNumCubeCorners = 8;

  struct BBox {
    enum Face {
      LEFT = 0,
      RIGHT,
      BOTTOM,
      TOP,
      BACK,
      FRONT
    };

    glm::vec3 min{ 0.f, 0.f, 0.f };
    glm::vec3 max{ 0.f, 0.f, 0.f };
    glm::vec3 extent{ 0.f, 0.f, 0.f };

    std::array<glm::vec3, kNumCubeCorners> corners = {};

    BBox() {}
    BBox(const glm::vec3& point);
    BBox(Interval x, Interval y, Interval z);
    BBox(const glm::vec3& min, const glm::vec3& max);
    BBox(const BBox& a, const BBox& b);

    bool Contains(const glm::vec3& point) const;
    bool OnBoundary(const glm::vec3& point) const;

    bool Hit(const Ray& ray, Interval& trace_interval) const;
    glm::vec3 GetFaceNormal(const glm::vec3& point, const glm::vec3& ray_dir) const;

    void ExpandToInclude(const glm::vec3& point);
    void ExpandToFill(const BBox& other);

    glm::vec3 Center() const;

    float MaxDimension() const;
    float SurfaceArea() const;

    const std::array<glm::vec3, 8>& Corners() const;

    static BBox Union(const BBox& a, const BBox& b);

    static const BBox empty;
    static const BBox universe;

   private:
    void CalculateCorners();

    Interval AxisInterval(uint32_t axis) const;
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

ECHO_TYPE(
  type(other::BBox),
  field(min),
  field(max),
  field(extent),
  field(corners)
)

#endif  // !OTHER_ENGINE_BOUNDING_BOX_HPP
