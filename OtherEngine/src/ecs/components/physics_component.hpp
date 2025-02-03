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

  struct RigidBody;
  struct Collider;
  struct PhysicsObject;

  struct RigidBodySnapshotter : public ObjectSerializer<RigidBody, 13> {
    RigidBodySnapshotter();

    static size_t Stride() {
      return sizeof(PhysicsBodyType) + sizeof(uint32_t) + sizeof(bool) * 3 + sizeof(float) * 5 + sizeof(glm::vec3) * 2 + sizeof(CollisionDetectionType);
    }
  };

  struct ColliderSnapshotter : public ObjectSerializer<Collider, 2> {
    ColliderSnapshotter();

    static size_t Stride() {
      return sizeof(uint32_t) + sizeof(glm::vec3);
    }
  };

  struct PhysicsObjectSnapshotter : public ObjectSerializer<PhysicsObject, 0> {
    PhysicsObjectSnapshotter() {}

    virtual void Write(ByteBuffer& stream, const PhysicsObject& object) override;
    virtual PhysicsObject Read(ByteBuffer& stream, size_t buffer_offset) override;

    /// this is zero because ColliderSnapshotter and RigidBodySnapshotter will each return their own stride separately
    static size_t Stride() { return 0; /* RigidBodySnapshotter::Stride() + ColliderSnapshotter::Stride(); */ }
  };

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

  struct ColliderSnapshotter;

  struct Collider : public Component {
    uint32_t shape_idx = PhysicsShape::Shape::BOX;
    glm::vec3 collider_scale = glm::vec3(1.f);

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
  field(type, echo::serializable_field()),
  field(layer_id, echo::serializable_field()),
  field(enable_dynamic_type_change, echo::serializable_field()),
  field(mass, echo::serializable_field()),
  field(linear_drag, echo::serializable_field()),
  field(angular_drag, echo::serializable_field()),
  field(disable_gravity, echo::serializable_field()),
  field(is_trigger, echo::serializable_field()),
  field(collision_type, echo::serializable_field()),
  field(initial_linear_velocity, echo::serializable_field()),
  field(initial_angular_velocity, echo::serializable_field()),
  field(max_linear_velocity, echo::serializable_field()),
  field(max_angular_velocity, echo::serializable_field())
);

ECHO_TYPE(
  type(other::Collider, refl::attr::bases<other::Component>),
  field(shape_idx),
  field(collider_scale)
);

ECHO_TYPE(
  type(other::PhysicsObject, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_PHYSICS_COMPONENT_HPP