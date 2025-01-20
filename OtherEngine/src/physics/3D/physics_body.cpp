
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

  void PhysicsBody::SetNativeBody(void* body) {
    native_body = body;
  }

  void PhysicsBody::SetEntityID(const UUID& id) {
    entity_id = id;
    OnSetEntity();
  }

  void PhysicsBody::AddCollider(Ref<PhysicsShape> shape) {
    if (collider_shape != nullptr) {
      /// destroy collider shape
      RemoveCollider(shape);
    }

    /// add new collider shape
    collider_shape = shape;
    OnAddCollider(shape);
  }

  void PhysicsBody::RemoveCollider(Ref<PhysicsShape> shape) {
    if (collider_shape == nullptr) {
      return;
    }

    if (collider_shape->NativeShape() != shape->NativeShape()) {
      return;
    }

    // physics_world->UnregisterColliderShape(entity_id, collider_shape);

    OnRemoveCollider(shape);
    collider_shape = nullptr;
  }

  PhysicsBodyType PhysicsBody::GetType() const {
    return body_type;
  }

  uint32_t PhysicsBody::GetLayer() const {
    return object_layer;
  }

}  // namespace other
