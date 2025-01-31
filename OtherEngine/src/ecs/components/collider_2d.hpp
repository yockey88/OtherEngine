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

  class Collider2DSnapshotter : public ObjectSerializer<Collider2D, 4> {
   public:
    Collider2DSnapshotter() {
      AddField<glm::vec2, 0>(&Collider2D::offset);
      AddField<glm::vec2, 1>(&Collider2D::size);
      AddField<float, 2>(&Collider2D::density);
      AddField<float, 3>(&Collider2D::friction);
    }
  };

}  // namespace other

ECHO_TYPE(
  type(other::Collider2D, refl::attr::bases<other::Component>),
  field(offset),
  field(size),
  field(density),
  field(friction)
);

#endif  // !OTHER_ENGINE_COLLIDER_2D_HPP
