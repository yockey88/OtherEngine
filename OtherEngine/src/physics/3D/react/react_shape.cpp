/**
 * \file physics/3D/react/react_shape.pp
 **/
#include "physics/3D/react/react_shape.hpp"

#include "physics/3D/react/react_world.hpp"

namespace other {

  void ReactBoxShape::OnSetEntity(const UUID& id) {
  }

  glm::vec2 ReactBoxShape::HalfExtents() const {
    rp3d::Vector3 extents = shape->getHalfExtents();
    return glm::vec2(extents.x, extents.y);
  }

  void ReactSphereShape::OnSetEntity(const UUID& id) {
  }

  float ReactSphereShape::Radius() const {
    return shape->getRadius();
  }

  void ReactCapsuleShape::OnSetEntity(const UUID& id) {
  }

  float ReactCapsuleShape::Radius() const {
    return shape->getRadius();
  }

  float ReactCapsuleShape::Height() const {
    return shape->getHeight();
  }

  void ReactConvexMeshShape::OnSetEntity(const UUID& id) {
  }

}  // namespace other