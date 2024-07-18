/**
 * \file physics/2D/physics_world_2d.cpp
 **/
#include "physics/2D/physics_world_2d.hpp"

namespace other {

  PhysicsWorld2D::PhysicsWorld2D(const glm::vec2& grav) {
    gravity = {
      grav.x , grav.y
    };

    world = NewScope<b2World>(gravity);
  }

  void PhysicsWorld2D::Step(float dt , uint32_t vel_iters , uint32_t pos_iters) {
    world->Step(dt , vel_iters , pos_iters);
  }

  b2Body* PhysicsWorld2D::CreateBody(const b2BodyDef* body_def) {
    return world->CreateBody(body_def);
  }
      
  void PhysicsWorld2D::DestroyBody(b2Body* body) {
    world->DestroyBody(body);
  }
      
  glm::vec2 PhysicsWorld2D::GetGravity() const {
    return { gravity.x , gravity.y };
  }

} // namespace other
