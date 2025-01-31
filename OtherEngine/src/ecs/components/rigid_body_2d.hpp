/**
 * \file ecs/components/rigid_body_2d.hpp
 **/
#ifndef OTHER_ENGINE_RIGID_BODY_2D_HPP
#define OTHER_ENGINE_RIGID_BODY_2D_HPP

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  struct RigidBody2D : public Component {
    b2BodyDef body_def;
    b2Body* physics_body = nullptr;
    b2MassData mass_data;

    PhysicsBodyType type = STATIC;
    float mass = 1.f;
    float linear_drag = 0.01f;
    float angular_drag = 0.05f;
    float gravity_scale = 1.f;

    bool fixed_rotation = false;
    bool bullet = false;

    ECS_COMPONENT(RigidBody2D, RIGIDBODY2D_COMPONENT_INDEX);
  };

  class RigidBody2DSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(RigidBody2D);
  };

  class RigidBody2DSnapshotter : public ObjectSerializer<RigidBody2D, 10> {
   public:
    RigidBody2DSnapshotter() {
      AddField<b2BodyDef, 0>(&RigidBody2D::body_def);
      AddField<b2Body*, 1>(&RigidBody2D::physics_body);
      AddField<b2MassData, 2>(&RigidBody2D::mass_data);
      AddField<PhysicsBodyType, 3>(&RigidBody2D::type);
      AddField<float, 4>(&RigidBody2D::mass);
      AddField<float, 5>(&RigidBody2D::linear_drag);
      AddField<float, 6>(&RigidBody2D::angular_drag);
      AddField<float, 7>(&RigidBody2D::gravity_scale);
      AddField<bool, 8>(&RigidBody2D::fixed_rotation);
      AddField<bool, 9>(&RigidBody2D::bullet);
    }
  };

}  // namespace other

ECHO_TYPE(
  type(other::RigidBody2D, refl::attr::bases<other::Component>),
  field(mass),
  field(linear_drag),
  field(angular_drag),
  field(gravity_scale),
  field(fixed_rotation),
  field(bullet)
);

#endif  // !OTHER_ENGINE_RIGID_BODY_2D_HPP
