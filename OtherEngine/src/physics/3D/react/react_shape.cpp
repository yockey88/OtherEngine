/**
 * \file physics/3D/react/react_shape.pp
 **/
#include "physics/3D/react/react_shape.hpp"

#include "physics/3D/react/react_world.hpp"

namespace other {

  glm::vec2 ReactBoxShape::HalfExtents() const {
    rp3d::Vector3 extents = shape->getHalfExtents();
    return glm::vec2(extents.x, extents.y);
  }

  float ReactSphereShape::Radius() const {
    return shape->getRadius();
  }

  float ReactCapsuleShape::Radius() const {
    return shape->getRadius();
  }

  float ReactCapsuleShape::Height() const {
    return shape->getHeight();
  }

}  // namespace other