/**
 * \file ecs/components/rigid_body_2d.hpp
 **/
#ifndef OTHER_ENGINE_RIGID_BODY_2D_HPP
#define OTHER_ENGINE_RIGID_BODY_2D_HPP

#include <box2d/b2_body.h>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  struct RigidBody2D : public Component {
    b2Body* physics_body = nullptr; 
    PhysicsBodyType type;
    float mass = 1.f;
    float linear_drag = 0.01f;
    float angular_drag = 0.05f;
    float gravity_scale = 1.f;

    bool fixed_rotation = false;
    bool bullet = false;

    ECS_COMPONENT(RigidBody2D , kRigidBody2DIndex);
  }; 

  class RigidBody2DSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(RigidBody2D);
  };

} // namespace other

#endif // !OTHER_ENGINE_RIGID_BODY_2D_HPP
