
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

}  // namespace other
