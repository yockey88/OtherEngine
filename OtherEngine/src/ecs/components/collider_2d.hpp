/**
 * \file ecs/components/collider_2d.hpp
 **/
#ifndef OTHER_ENGINE_COLLIDER_2D_HPP
#define OTHER_ENGINE_COLLIDER_2D_HPP

#include <glm/glm.hpp>
#include <box2d/box2d.h>

#include "ecs/component.hpp"
#include "ecs/component_serializer.hpp"

namespace other {

  struct Collider2D : Component {
    glm::vec2 offset = { 0.f , 0.f };
    glm::vec2 size = { 0.5f , 0.5f };

    float density = 1.f;
    float friction = 1.f;

    b2Fixture* fixture = nullptr;

    ECS_COMPONENT(Collider2D , kCollider2DIndex);
  }; 

  class Collider2DSerializer : public ComponentSerializer {
    public:
      COMPONENT_SERIALIZERS(Collider2D);
  };

} // namespace other

#endif // !OTHER_ENGINE_COLLIDER_2D_HPP
