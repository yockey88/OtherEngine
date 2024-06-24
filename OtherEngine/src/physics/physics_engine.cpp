/**
 * \file physics/physics_engine.cpp
 **/
#include "physics/phyics_engine.hpp"

#include "core/logger.hpp"

#include "physics/2D/physics_world_2d.hpp"

namespace other {
      
  Ref<PhysicsWorld2D> PhysicsEngine::GetPhysicsWorld(const glm::vec2& gravity) {
    return Ref<PhysicsWorld2D>::Create(gravity);
  }


} // namespace other
