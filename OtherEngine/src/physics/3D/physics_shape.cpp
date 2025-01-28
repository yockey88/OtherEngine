
/**
 * \file physics/3D/physics_shape.cpp
 **/
#include "physics/3D/physics_shape.hpp"

#include "core/logger.hpp"

namespace other {

  void PhysicsShape::SetEntity(const UUID& id) {
    entity_id = id;
    OnSetEntity(id);
  }

  PhysicsShape::Shape PhysicsShape::ShapeType() const {
    return type;
  }

  void CompoundShape::OnSetEntity(const UUID& id) {
    for (auto& shape : shapes) {
      OE_ASSERT(shape != nullptr, "Physics shape is null");
      shape->SetEntity(id);
    }
  }

  void CompoundShape::SetTransform(const Transform& transform) {
    for (auto& shape : shapes) {
      OE_ASSERT(shape != nullptr, "Physics shape is null");
      shape->SetTransform(transform);
    }
  }

  void CompoundShape::SetScale(const glm::vec3& scale) {
    for (auto& shape : shapes) {
      OE_ASSERT(shape != nullptr, "Physics shape is null");
      shape->SetScale(scale);
    }
  }

}  // namespace other
