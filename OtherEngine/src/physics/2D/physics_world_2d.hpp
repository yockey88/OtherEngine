/**
 * \file physics/2D/physics_world_2d.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_WORLD_2D_HPP
#define OTHER_ENGINE_PHYSICS_WORLD_2D_HPP

#include <glm/glm.hpp>
#include <box2d/b2_world.h>

#include "core/defines.hpp"
#include "core/ref_counted.hpp"

namespace other {

  class PhysicsWorld2D : public RefCounted {
    public:
      PhysicsWorld2D(const glm::vec2& grav);

      void Step(float dt , uint32_t vel_iters , uint32_t pos_iters);

      b2Body* CreateBody(const b2BodyDef* body_def);
      void DestroyBody(b2Body* body);

      glm::vec2 GetGravity() const;

    private:
      b2Vec2 gravity;
      Scope<b2World> world = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_WORLD_2D_HPP
