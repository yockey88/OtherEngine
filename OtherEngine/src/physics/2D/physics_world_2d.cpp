/**
 * \file physics/2D/physics_world_2d.cpp
 **/
#include "physics/2D/physics_world_2d.hpp"

namespace other {
      
  PhysicsWorld2D::PhysicsWorld2D(const glm::vec2& gravity) {
    this->gravity = b2Vec2{
      gravity.x ,
      gravity.y ,
    };

    world = NewScope<b2World>(this->gravity);
  }

  void PhysicsWorld2D::Step(float dt , uint32_t velocity_iters , uint32_t pos_iters) {
    world->Step(dt , velocity_iters , pos_iters);
  }
      
  Ref<PhysicsBody2D> PhysicsWorld2D::CreateBody(const b2BodyDef* body_definition) {
    return Ref<PhysicsBody2D>::Create(world , body_definition);
  }
      
  glm::vec2 PhysicsWorld2D::GetGravity() {
    return { gravity.x , gravity.y };
  }

} // namespace other
