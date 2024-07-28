
/**
 * \file ecs/components/rigid_body.hpp
 **/
#ifndef OTHER_ENGINE_RIGID_BODY_HPP
#define OTHER_ENGINE_RIGID_BODY_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct RigidBody : public Component {
    ECS_COMPONENT(RigidBody , kRigidBodyIndex);
  };

  class RigidBodySerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(RigidBody);
  };

} // namespace other

#endif // !OTHER_ENGINE_RIGID_BODY_HPP
