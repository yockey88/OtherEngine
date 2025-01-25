/**
 * \file ecs/components/physics_component.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_COMPONENT_HPP
#define OTHER_ENGINE_PHYSICS_COMPONENT_HPP

#include "core/ref.hpp"

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "physics/3D/physics_body.hpp"
#include "physics/physics_defines.hpp"

namespace other {

  class PhysicsWorld;

  /// this has a ton in common with the 2D version so it might be worth it
  ///   to merge them into one
  struct RigidBody : public Component {
    PhysicsBodyType type = DYNAMIC;
    uint32_t layer_id = 0;  /// Jolt layer id
    bool enable_dynamic_type_change = false;

    float mass = 1.f;
    float linear_drag = 0.01f;
    float angular_drag = 0.05f;

    bool disable_gravity = false;
    bool is_trigger = false;

    CollisionDetectionType collision_type = DISCRETE_COLLISION;

    glm::vec3 initial_linear_velocity{ 0.f };
    glm::vec3 initial_angular_velocity{ 0.f };

    float max_linear_velocity = 500.f;
    float max_angular_velocity = 50.f;

    Ref<PhysicsBody> physics_body = nullptr;

    ECS_COMPONENT(RigidBody, RIGIDBODY_COMPONENT_INDEX);
  };

  class RigidBodySerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(RigidBody);

    bool force_deserialize = false;
  };

  struct Collider : public Component {
    uint32_t shape_idx = PhysicsShape::Shape::BOX;
    Ref<PhysicsShape> shape = nullptr;
    ECS_COMPONENT(Collider, COLLIDER_COMPONENT_INDEX);
    Collider(uint32_t shape_idx)
        : Component(COLLIDER_COMPONENT_INDEX), shape_idx(shape_idx) {}
  };

  class ColliderSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Collider);

    bool force_deserialize = false;
  };

  struct PhysicsObject : public Component {
    ECS_COMPONENT(PhysicsObject, PHYSICS_OBJECT_COMPONENT_INDEX);
  };

  class PhysicsObjectSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(PhysicsObject);
  };

}  // namespace other

ECHO_TYPE(
  type(other::RigidBody, refl::attr::bases<other::Component>),
  field(type),
  field(layer_id),
  field(enable_dynamic_type_change),
  field(mass),
  field(linear_drag),
  field(angular_drag),
  field(disable_gravity),
  field(is_trigger),
  field(collision_type),
  field(initial_linear_velocity),
  field(initial_angular_velocity),
  field(max_linear_velocity),
  field(max_angular_velocity)
);

ECHO_TYPE(
  type(other::Collider, refl::attr::bases<other::Component>)
);

ECHO_TYPE(
  type(other::PhysicsObject, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_PHYSICS_COMPONENT_HPP