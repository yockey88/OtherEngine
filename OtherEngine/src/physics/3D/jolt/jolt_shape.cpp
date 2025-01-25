/**
 * \file physics/3D/jolt/jolt_shape.cpp
 **/
#include "physics/3D/jolt/jolt_shape.hpp"

#include <Jolt/Math/Real.h>

namespace other {

  void JoltBoxShape::OnSetEntity(const UUID& id) {
    rotated_translated_shape->SetUserData(id.Get());
  }

  void JoltBoxShape::SetTransform(const Transform& transform) {
    JPH::RVec3 position(transform.position.x, transform.position.y, transform.position.z);
    JPH::Quat orientation(transform.qrotation.x, transform.qrotation.y, transform.qrotation.z, transform.qrotation.w);
  }

  glm::vec2 JoltBoxShape::HalfExtents() const {
    return glm::vec2();
  }

  void JoltSphereShape::OnSetEntity(const UUID& id) {
  }

  void JoltSphereShape::SetTransform(const Transform& transform) {
  }

  float JoltSphereShape::Radius() const {
    return 0.0f;
  }

  void JoltCapsuleShape::OnSetEntity(const UUID& id) {
  }

  void JoltCapsuleShape::SetTransform(const Transform& transform) {
  }

  float JoltCapsuleShape::Radius() const {
    return 0.0f;
  }

  float JoltCapsuleShape::Height() const {
    return 0.0f;
  }

  void JoltConvexMeshShape::OnSetEntity(const UUID& id) {
  }

  void JoltConvexMeshShape::SetTransform(const Transform& transform) {
  }

  void JoltConcaveMeshShape::OnSetEntity(const UUID& id) {
  }

  void JoltConcaveMeshShape::SetTransform(const Transform& transform) {
  }

}  // namespace other