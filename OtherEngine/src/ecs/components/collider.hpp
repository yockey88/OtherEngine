
/**
 * \file ecs/components/collider.hpp
 **/
#ifndef OTHER_ENGINE_COLLIDER_HPP
#define OTHER_ENGINE_COLLIDER_HPP

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Collider : public Component {
    ECS_COMPONENT(Collider , kRigidBodyIndex);
  };

  class ColliderSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Collider);
  };

} // namespace other

#endif // !OTHER_ENGINE_COLLIDER_HPP
