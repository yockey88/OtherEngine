/**
 * \file math/bounding_box.cpp
 **/
#include "math/bounding_box.hpp"

#include <glm/fwd.hpp>

#include "core/logger.hpp"
#include "math/vecmath.hpp"

namespace other {

  const BBox BBox::empty = BBox(Interval::empty, Interval::empty, Interval::empty);
  const BBox BBox::universe = BBox(Interval::universe, Interval::universe, Interval::universe);

  BBox::BBox(const glm::vec3& point) : BBox(point, point) {
  }

  BBox::BBox(Interval x, Interval y, Interval z) {
    min = { x.min, y.min, z.min };
    max = { x.max, y.max, z.max };
    extent = vec3_sub(max, min);
    CalculateCorners();
  }

  BBox::BBox(const glm::vec3& min, const glm::vec3& max)
      : min(min), max(max), extent(vec3_sub(max, min)) {
    CalculateCorners();
  }

  BBox::BBox(const BBox& a, const BBox& b) {
    if (vec3_gt(a.max, b.max)) {
      max = a.max;
    } else {
      max = b.max;
    }

    if (vec3_lt(a.min, b.min)) {
      min = a.min;
    } else {
      min = b.min;
    }

    extent = vec3_sub(max, min);
  }

  bool BBox::Contains(const glm::vec3& point) const {
    return vec3_gte(point, min) && vec3_lte(point, max);
  }

  bool BBox::OnBoundary(const glm::vec3& point) const {
    return EpsilonEqual(point.x, min.x) || EpsilonEqual(point.x, max.x) ||
      EpsilonEqual(point.y, min.y) || EpsilonEqual(point.y, max.y) ||
      EpsilonEqual(point.z, min.z) || EpsilonEqual(point.z, max.z);
  }

  bool BBox::Hit(const Ray& ray, Interval& trace_interval) const {
    const glm::vec3& origin = ray.origin;
    const glm::vec3& direction = ray.direction;

    glm::vec3 inv_direction = {
      EpsilonQuotient(1, direction.x),
      EpsilonQuotient(1, direction.y),
      EpsilonQuotient(1, direction.z)
    };

    float t1 = EpsilonProduct(EpsilonSubtract(min.x, origin.x), inv_direction.x);
    float t2 = EpsilonProduct(EpsilonSubtract(max.x, origin.x), inv_direction.x);

    float t3 = EpsilonProduct(EpsilonSubtract(min.y, origin.y), inv_direction.y);
    float t4 = EpsilonProduct(EpsilonSubtract(max.y, origin.y), inv_direction.y);

    float t5 = EpsilonProduct(EpsilonSubtract(min.z, origin.z), inv_direction.z);
    float t6 = EpsilonProduct(EpsilonSubtract(max.z, origin.z), inv_direction.z);

    float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
    float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

    if (tmax < 0) {
      trace_interval.min = tmax;
      return false;
    }

    if (tmin > tmax) {
      trace_interval.min = tmax;
      return false;
    }

    trace_interval.min = tmin;
    trace_interval.max = tmax;
    return true;
  }

  glm::vec3 BBox::GetFaceNormal(const glm::vec3& point, const glm::vec3& ray_dir) const {
    std::array<glm::vec3, 4> face_vertices = {};

    if (EpsilonEqual(point.x, min.x)) {
      face_vertices = { corners[1], corners[2], corners[6], corners[5] };
    } else if (EpsilonEqual(point.x, max.x)) {
      face_vertices = { corners[0], corners[3], corners[7], corners[4] };
    } else if (EpsilonEqual(point.y, min.y)) {
      face_vertices = { corners[2], corners[3], corners[7], corners[6] };
    } else if (EpsilonEqual(point.y, max.y)) {
      face_vertices = { corners[0], corners[1], corners[5], corners[4] };
    } else if (EpsilonEqual(point.z, min.z)) {
      face_vertices = { corners[4], corners[5], corners[6], corners[7] };
    } else if (EpsilonEqual(point.z, max.z)) {
      face_vertices = { corners[0], corners[1], corners[2], corners[3] };
    }

    glm::vec3 v0 = vec3_sub(face_vertices[1], face_vertices[0]);
    glm::vec3 v1 = vec3_sub(face_vertices[2], face_vertices[0]);
    glm::vec3 normal = glm::cross(v0, v1);

    glm::vec3 center_to_point = vec3_sub(point, Center());
    if (glm::dot(normal, center_to_point) < 0.f) {
      normal = vec3_product(-1.f, normal);
    }

    return glm::normalize(normal);
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

  BBox BBox::Union(const BBox& a, const BBox& b) {
    BBox result;
    result.min = {
      glm::min(a.min.x, b.min.x),
      glm::min(a.min.y, b.min.y),
      glm::min(a.min.z, b.min.z)
    };
    result.max = {
      glm::max(a.max.x, b.max.x),
      glm::max(a.max.y, b.max.y),
      glm::max(a.max.z, b.max.z)
    };
    result.extent = vec3_sub(result.max, result.min);
    return result;
  }

  void BBox::CalculateCorners() {
    corners = {
      max,
      { min.x, max.y, max.z },
      { min.x, min.y, max.z },
      { max.x, min.y, max.z },

      { max.x, max.y, min.z },
      { min.x, max.y, min.z },
      min,
      { max.x, min.y, min.z },
    };
  }

  Interval BBox::AxisInterval(uint32_t axis) const {
    switch (axis) {
      case 0:
        return Interval(min.x, max.x);
      case 1:
        return Interval(min.y, max.y);
      case 2:
        return Interval(min.z, max.z);
    }
    OE_ASSERT(false, "Invalid Axis : {}", axis);
    return Interval::universe;
  }

}  // namespace other
