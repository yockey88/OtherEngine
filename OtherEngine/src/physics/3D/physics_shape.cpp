
/**
 * \file physics/3D/physics_shape.cpp
 **/
#include "physics/3D/physics_shape.hpp"

namespace other {

  void PhysicsShape::SetEntity(const UUID& id) {
    entity_id = id;
  }

  PhysicsShape::Shape PhysicsShape::ShapeType() const {
    return type;
  }

}  // namespace other
