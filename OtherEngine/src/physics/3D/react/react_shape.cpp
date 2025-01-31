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
    SetScale(scale);
  }

  void ReactBoxShape::SetScale(const glm::vec3& scale) {
    rp3d::Vector3 half_extents = {
      scale.x / 2.f,
      scale.y / 2.f,
      scale.z / 2.f,
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
    SetScale(transform.scale);
  }

  void ReactSphereShape::SetScale(const glm::vec3& scale) {
    shape->setRadius(scale.x / 2.f);
  }

  float ReactSphereShape::Radius() const {
    return shape->getRadius();
  }

  void ReactCapsuleShape::OnSetEntity(const UUID& id) {
  }

  void ReactCapsuleShape::SetTransform(const Transform& transform) {
    SetScale(transform.scale);
  }

  void ReactCapsuleShape::SetScale(const glm::vec3& scale) {
    shape->setRadius(scale.x / 2.f);
    shape->setHeight(scale.y);
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
    SetScale(transform.scale);
  }

  void ReactConvexMeshShape::SetScale(const glm::vec3& scale) {
    glm::vec3 half_scale = scale / 2.f;
    glm::vec3 new_min = -half_scale;
    glm::vec3 new_max = half_scale;

    rp3d::Vector3 min = {
      new_min.x,
      new_min.y,
      new_min.z,
    };

    rp3d::Vector3 max = {
      new_max.x,
      new_max.y,
      new_max.z,
    };

    shape->getLocalBounds().setMin(min);
    shape->getLocalBounds().setMax(max);
  }

  void ReactConcaveMeshShape::OnSetEntity(const UUID& id) {
  }

  void ReactConcaveMeshShape::SetTransform(const Transform& transform) {
    SetScale(transform.scale);
  }

  void ReactConcaveMeshShape::SetScale(const glm::vec3& scale) {
    glm::vec3 half_scale = scale / 2.f;
    glm::vec3 new_min = -half_scale;
    glm::vec3 new_max = half_scale;

    rp3d::Vector3 min = {
      new_min.x,
      new_min.y,
      new_min.z,
    };

    rp3d::Vector3 max = {
      new_max.x,
      new_max.y,
      new_max.z,
    };

    shape->getLocalBounds().setMin(min);
    shape->getLocalBounds().setMax(max);
  }

  void ReactCompoundShape::OnSetEntity(const UUID& id) {
  }

  void ReactCompoundShape::SetTransform(const Transform& transform) {
    SetScale(transform.scale);
  }

  void ReactCompoundShape::SetScale(const glm::vec3& scale) {
    glm::vec3 half_scale = scale / 2.f;
    glm::vec3 new_min = -half_scale;
    glm::vec3 new_max = half_scale;

    rp3d::Vector3 min = {
      new_min.x,
      new_min.y,
      new_min.z,
    };

    rp3d::Vector3 max = {
      new_max.x,
      new_max.y,
      new_max.z,
    };
  }

  void* ReactCompoundShape::NativeShape() {
    if (shapes.size() == 1) {
      return shapes.begin()->second;
    }
    return nullptr;
  }

  void ReactCompoundShape::AddShape(UUID id, rp3d::CollisionShape* shape) {
    if (shapes.find(id) != shapes.end()) {
      return;
    }

    shapes[id] = shape;
  }

  void ReactTerrainShape::OnSetEntity(const UUID& id) {
  }

  void ReactTerrainShape::SetTransform(const Transform& transform) {
    SetScale(transform.scale);
  }

  void ReactTerrainShape::SetScale(const glm::vec3& scale) {
    glm::vec3 half_scale = scale / 2.f;
    glm::vec3 new_min = -half_scale;
    glm::vec3 new_max = half_scale;

    rp3d::Vector3 min = {
      new_min.x,
      new_min.y,
      new_min.z,
    };

    rp3d::Vector3 max = {
      new_max.x,
      new_max.y,
      new_max.z,
    };

    shape->getLocalBounds().setMin(min);
    shape->getLocalBounds().setMax(max);
  }

}  // namespace other