/**
 * \file math/bounding_box.cpp
 **/
#include "math/bounding_box.hpp"

#include "core/logger.hpp"
#include "math/vecmath.hpp"

namespace other {

  bool BBox::Contains(const glm::vec3& point) const {
    return vec3_gte(point, min) && vec3_lte(point, max);
  }

  bool BBox::OnBoundary(const glm::vec3& point) const {
    return EpsilonEqual(point.x, min.x) || EpsilonEqual(point.x, max.x) ||
           EpsilonEqual(point.y, min.y) || EpsilonEqual(point.y, max.y) ||
           EpsilonEqual(point.z, min.z) || EpsilonEqual(point.z, max.z);
  }

  Intersection BBox::Intersect(const Ray& ray) const {
    Intersection intersection;

    return intersection;
  }

  void BBox::ExpandToInclude(const glm::vec3& point) {
    if (Contains(point)) {
      return;
    }

    if (OnBoundary(point)) {
      /// some arbitrary small value
      min = vec3_min(min, vec3_sum(point, 1000 * eps<float>()));
      max = vec3_max(max, vec3_sub(point, 1000 * eps<float>()));
      extent = vec3_diff(max, min);
      OE_ASSERT(!OnBoundary(point), "Point {} still on boundary of bounding box {}", point, *this);
      OE_ASSERT(Contains(point), "Point {} not contained in bounding box {}", point, *this);
    } else {
      min = vec3_min(min, point);
      max = vec3_max(max, point);
      extent = vec3_diff(max, min);
    }
  }

  void BBox::ExpandToFill(const BBox& other) {
    min = vec3_min(min, other.min);
    max = vec3_max(max, other.max);
    extent = vec3_diff(max, min);
  }

  glm::vec3 BBox::Center() const {
    if (vec3_eq(min, max)) {
      return min;
    }

    /// take the unit vector pointing from min to max and scale it by half the length of the extent
    //   to get the center of the bounding box
    float extent_length = glm::length(extent);
    glm::vec3 min_to_max = glm::normalize(extent);

    /// find vector pointing from min to center then add it to min
    glm::vec3 offset = vec3_product((extent_length / 2.f), min_to_max);
    glm::vec3 center = vec3_sum(min, offset);

    return center;
  }

  float BBox::MaxDimension() const {
    uint32_t result = 0;
    if (extent.y > extent.x) {
      result = 1;
    }

    if (extent.z > result == 0 ? extent.x : extent.y) {
      result = 2;
    }

    return extent[result];
  }

  float BBox::SurfaceArea() const {
    /// SA = 2(wl + hl + hw)
    return 2 * (extent.x * extent.y + extent.x * extent.z + extent.y * extent.z);
  }

  const std::array<glm::vec3, 8>& BBox::Corners() const {
    return corners;
  }

  void BBox::CalculateCorners() {
    corners = {
      max,
      {min.x, max.y, max.z},
      {min.x, min.y, max.z},
      {max.x, min.y, max.z},

      {max.x, max.y, min.z},
      {min.x, max.y, min.z},
      min,
      {max.x, min.y, min.z},
    };
  }

}  // namespace other