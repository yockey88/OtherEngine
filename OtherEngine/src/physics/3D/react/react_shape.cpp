/**
 * \file physics/3D/react/react_shape.pp
 **/
#include "physics/3D/react/react_shape.hpp"

#include "physics/3D/react/react_world.hpp"

namespace other {

  void ReactBoxShape::OnSetEntity(const UUID& id) {
  }

  void ReactBoxShape::SetTransform(const Transform& transform) {
    glm::vec3 scale = transform.scale / 2.f;
    rp3d::Vector3 half_extents = {
      scale.x,
      scale.y,
      scale.z,
    };
    shape->setHalfExtents(half_extents);
  }

  glm::vec2 ReactBoxShape::HalfExtents() const {
    rp3d::Vector3 extents = shape->getHalfExtents();
    return glm::vec2(extents.x, extents.y);
  }

  void ReactSphereShape::OnSetEntity(const UUID& id) {
  }

  void ReactSphereShape::SetTransform(const Transform& transform) {
    shape->setRadius(transform.scale.x / 2.f);
  }

  float ReactSphereShape::Radius() const {
    return shape->getRadius();
  }

  void ReactCapsuleShape::OnSetEntity(const UUID& id) {
  }

  void ReactCapsuleShape::SetTransform(const Transform& transform) {
    shape->setRadius(transform.scale.x / 2.f);
    shape->setHeight(transform.scale.y);
  }

  float ReactCapsuleShape::Radius() const {
    return shape->getRadius();
  }

  float ReactCapsuleShape::Height() const {
    return shape->getHeight();
  }

  void ReactConvexMeshShape::OnSetEntity(const UUID& id) {
  }

  void ReactConvexMeshShape::SetTransform(const Transform& transform) {
  }

}  // namespace other