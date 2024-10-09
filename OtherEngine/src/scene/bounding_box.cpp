/**
 * \file scene/bounding_box.cpp
 **/
#include "scene/bounding_box.hpp"

#include "glm/gtc/epsilon.hpp"

#include "core/logger.hpp"
#include "math/vecmath.hpp"

namespace other {
  
  bool BoundingBox::Contains(const glm::vec3& point) const {
    return vec3_gte(point , min) && vec3_lte(point , max);
  }

  bool BoundingBox::OnBoundary(const glm::vec3& point) const {
    return EpsilonEqual(point.x , min.x) || EpsilonEqual(point.x , max.x) ||
           EpsilonEqual(point.y , min.y) || EpsilonEqual(point.y , max.y) ||
           EpsilonEqual(point.z , min.z) || EpsilonEqual(point.z , max.z);
  }

  void BoundingBox::ExpandToInclude(const glm::vec3& point) {
    if (Contains(point)) {
      OE_TRACE("Point {} is already contained in bounding box" , point);
      return;
    }

    if (OnBoundary(point)) {
      /// some arbitrary small value
      min = vec3_min(min , vec3_sum(point , 1000 * eps<float>()));
      max = vec3_max(max , vec3_sub(point , 1000 * eps<float>()));
      extent = vec3_diff(max , min);
      OE_ASSERT(!OnBoundary(point) , "Point {} still on boundary of bounding box {}" , point , *this);
      OE_ASSERT(Contains(point) , "Point {} not contained in bounding box {}" , point , *this);
    } else {
      min = vec3_min(min , point);
      max = vec3_max(max , point);
      extent = vec3_diff(max , min);
    }
  }

  void BoundingBox::ExpandToFill(const BoundingBox& other) {
    min = vec3_min(min , other.min);
    max = vec3_max(max , other.max);
    extent = vec3_diff(max , min);
  }

  glm::vec3 BoundingBox::Center() const {
    if (vec3_eq(min , max)) {
      return min;
    }

    /// take the unit vector pointing from min to max and scale it by half the length of the extent
    //   to get the center of the bounding box
    float extent_length = glm::length(extent);
    glm::vec3 min_to_max = glm::normalize(extent);

    /// find vector pointing from min to center then add it to min
    glm::vec3 offset = vec3_product((extent_length / 2.f) , min_to_max);
    glm::vec3 center = vec3_sum(min , offset);

    return center;
  }

  uint32_t BoundingBox::MaxDimension() const {
    uint32_t result = 0;
    if (extent.y > extent.x) {
      result = 1;
    }

    float dim = result == 0 ? extent.x : extent.y;
    if (extent.z > dim) {
      result = 2;
    }

    return result;
  }
  
  const std::array<glm::vec3 , 8>& BoundingBox::Corners() {
    if (!corners.has_value()) {
      corners = std::array<glm::vec3 , 8>{
        min,
        { min.x , max.y , min.z },
        { max.x , max.y , min.z },
        { max.x , min.y , min.z },
        { min.x , min.y , max.z },
        { min.x , max.y , max.z },
        max,
        { max.x , min.y , max.z }
      };
    }
    return corners.value();
  }

} // namespace other