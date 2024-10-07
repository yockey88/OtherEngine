/**
 * \file scene/bounding_box.cpp
 **/
#include "scene/bounding_box.hpp"

#include "core/logger.hpp"

namespace other {
  
  bool BoundingBox::Contains(const glm::vec3& point) const {
    return point.x >= min.x && point.x <= max.x &&
           point.y >= min.y && point.y <= max.y &&
           point.z >= min.z && point.z <= max.z;
  }

  void BoundingBox::ExpandToInclude(const glm::vec3& point) {
    if (Contains(point)) {
      return;
    }
    
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
    float extent_length = glm::length(extent);
    glm::vec3 min_to_max = nvec_norm<3>(extent);
    
    auto offset = (extent_length / 2.f) * min_to_max;
    auto center = nvec_sum<3>(min , offset);
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