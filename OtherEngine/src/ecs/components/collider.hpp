
/**
 * \file ecs/components/collider.hpp
 **/
#ifndef OTHER_ENGINE_COLLIDER_HPP
#define OTHER_ENGINE_COLLIDER_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

#include "physics/3D/physics_shape.hpp"

namespace other {

  struct Collider : public Component {
    Ref<PhysicsShape> shape = nullptr;
    ECS_COMPONENT(Collider, kRigidBodyIndex);
  };

  class ColliderSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Collider);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Collider, refl::attr::bases<other::Component>)
);

#endif  // !OTHER_ENGINE_COLLIDER_HPP
