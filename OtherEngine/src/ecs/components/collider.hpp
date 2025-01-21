
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
    uint32_t shape_idx = PhysicsShape::Shape::BOX;
    Ref<PhysicsShape> shape = nullptr;
    ECS_COMPONENT(Collider, kColliderIndex);
    Collider(uint32_t shape_idx)
        : Component(kColliderIndex), shape_idx(shape_idx) {}
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
