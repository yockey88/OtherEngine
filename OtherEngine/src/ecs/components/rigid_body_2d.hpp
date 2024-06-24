/**
 * \file ecs/components/rigid_body_2d.hpp
 **/
#ifndef OTHER_ENGINE_RIGID_BODY_2D_HPP
#define OTHER_ENGINE_RIGID_BODY_2D_HPP

#include <box2d/b2_body.h>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct RigidBody2D : public Component {
    b2Body* physics_body = nullptr; 

    ECS_COMPONENT(RigidBody2D , kRigidBody2DIndex);
  }; 

  class RigidBody2DSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(RigidBody2D);
  };

} // namespace other

#endif // !OTHER_ENGINE_RIGID_BODY_2D_HPP
