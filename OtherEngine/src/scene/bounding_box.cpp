/**
 * \file scene/bounding_box.cpp
 **/
#include "scene/bounding_box.hpp"

namespace other {
  
  void BoundingBox::ExpandToInclude(const glm::vec3& point) {
    min = glm::min(min , point);
    max = glm::max(max , point);
    extent = max - min;
  }

  void BoundingBox::ExpandToFill(const BoundingBox& other) {
    min = glm::min(min , other.min);
    max = glm::max(max , other.max);
    extent = max - min;
  }

  glm::vec3 BoundingBox::Center() const {
    return (max + min) * 0.5f;
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

} // namespace other