/**
 * \file physics/2D/physics_body_2d.cpp
 **/
#include "physics/2D/physics_body_2d.hpp"

namespace other {

  PhysicsBody2D::PhysicsBody2D(Scope<b2World>& world , const b2BodyDef* body_def) 
      : context(world) {
    body = world->CreateBody(body_def); 
  } 
      
  PhysicsBody2D::~PhysicsBody2D() {
    context->DestroyBody(body);
  }

} // namespace other
