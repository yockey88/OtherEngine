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
