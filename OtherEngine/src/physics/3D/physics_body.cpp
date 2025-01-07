
/**
 * \file physics/3D/physics_body.cpp
 **/
#include "physics/3D/physics_body.hpp"

namespace other {

  void PhysicsBody::SetType(PhysicsBodyType type) {
    body_type = type;
    OnBodyTypeChange(type);
  }

  void PhysicsBody::SetLayer(uint32_t layer) {
    object_layer = layer;
    OnLayerChange(layer);
  }

  PhysicsBodyType PhysicsBody::GetType() const {
    return body_type;
  }

  uint32_t PhysicsBody::GetLayer() const {
    return object_layer;
  }

}  // namespace other
