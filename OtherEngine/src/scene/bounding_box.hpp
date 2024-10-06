/**
 * \file scene/bounding_box.hpp
 **/
#ifndef OTHER_ENGINE_BOUNDING_BOX_HPP
#define OTHER_ENGINE_BOUNDING_BOX_HPP

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

namespace other {

  struct BoundingBox {
    glm::vec3 min = glm::vec3(0.f);
    glm::vec3 max = glm::vec3(0.f);
    glm::vec3 extent = glm::vec3(0.f);

    constexpr BoundingBox() {}
    constexpr BoundingBox(const glm::vec3& min , const glm::vec3& max) : min(min) , max(max) , extent(max - min) {}
    constexpr BoundingBox(const glm::vec3& point) : BoundingBox(point , point) {}

    void ExpandToInclude(const glm::vec3& point);
    void ExpandToFill(const BoundingBox& other);

    glm::vec3 Center() const;
    bool Intersects(/* ray , near , far */) const { return false; }

    uint32_t MaxDimension() const;
    constexpr float SurfaceArea() const;
  };

} // namespace other

#endif // !OTHER_ENGINE_BOUNDING_BOX_HPP