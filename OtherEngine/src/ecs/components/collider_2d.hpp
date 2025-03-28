/**
 * \file ecs/components/collider_2d.hpp
 **/
#ifndef OTHER_ENGINE_COLLIDER_2D_HPP
#define OTHER_ENGINE_COLLIDER_2D_HPP

#include <box2d/box2d.h>
#include <glm/glm.hpp>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Collider2D;

  struct Collider2DSnapshotter : public ObjectSerializer<Collider2D, 4> {
    Collider2DSnapshotter();

    static size_t Stride() {
      return sizeof(glm::vec2) * 2 + sizeof(float) * 2;
    }
  };

  struct Collider2D : Component {
    glm::vec2 offset = { 0.f, 0.f };
    glm::vec2 size = { 0.5f, 0.5f };

    float density = 1.f;
    float friction = 1.f;

    b2Fixture* fixture = nullptr;

    ECS_COMPONENT(Collider2D, COLLIDER2D_COMPONENT_INDEX);
  };

  class Collider2DSerializer : public ComponentSerializer {
   public:
    COMPONENT_SERIALIZERS(Collider2D);
  };

}  // namespace other

ECHO_TYPE(
  type(other::Collider2D, refl::attr::bases<other::Component>),
  field(offset, echo::serializable_field()),
  field(size, echo::serializable_field()),
  field(density, echo::serializable_field()),
  field(friction, echo::serializable_field())
);

#endif  // !OTHER_ENGINE_COLLIDER_2D_HPP
