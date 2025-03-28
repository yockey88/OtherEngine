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

  void JoltBoxShape::SetScale(const glm::vec3& scale) {
    // JPH::RVec3 half_extents(scale.x / 2.f, scale.y / 2.f, scale.z / 2.f);
    // shape->SetHalfExtents(half_extents);
  }

  glm::vec2 JoltBoxShape::HalfExtents() const {
    return glm::vec2();
  }

  void JoltSphereShape::OnSetEntity(const UUID& id) {
  }

  void JoltSphereShape::SetTransform(const Transform& transform) {
  }

  void JoltSphereShape::SetScale(const glm::vec3& scale) {
  }

  float JoltSphereShape::Radius() const {
    return 0.0f;
  }

  void JoltCapsuleShape::OnSetEntity(const UUID& id) {
  }

  void JoltCapsuleShape::SetTransform(const Transform& transform) {
  }

  void JoltCapsuleShape::SetScale(const glm::vec3& scale) {
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

  void JoltConvexMeshShape::SetScale(const glm::vec3& scale) {
  }

  void JoltConcaveMeshShape::OnSetEntity(const UUID& id) {
  }

  void JoltConcaveMeshShape::SetTransform(const Transform& transform) {
  }

  void JoltConcaveMeshShape::SetScale(const glm::vec3& scale) {
  }

}  // namespace other