/**
 * \file physics/2D/physics_world_2d.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_WORLD_2D_HPP
#define OTHER_ENGINE_PHYSICS_WORLD_2D_HPP

#include <glm/glm.hpp>
#include <box2d/box2d.h>

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "physics/2D/physics_body_2d.hpp"

namespace other {
  
  class PhysicsWorld2D : public RefCounted {
    public:
      PhysicsWorld2D(const glm::vec2& gravity);

      void Step(float dt , uint32_t velocity_iters , uint32_t pos_iters);
      
      Ref<PhysicsBody2D> CreateBody(const b2BodyDef* body_definition);

      glm::vec2 GetGravity() const;

    private:
      b2Vec2 gravity = b2Vec2{ 0 , -10.f };
      Scope<b2World> world = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_WORLD_2D_HPP
