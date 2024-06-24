/**
 * \file physics/physics_engine.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_ENGINE_HPP
#define OTHER_ENGINE_PHYSICS_ENGINE_HPP

#include "core/ref.hpp"

#include "physics/2D/physics_world_2d.hpp"

namespace other {

  enum PhysicsType {
    PHYSICS_2D , 
    PHYSICS_3D ,

    NUM_PHYSICS_TYPES ,
    INVALID_PHYSICS_TYPE = NUM_PHYSICS_TYPES
  };

  class PhysicsEngine {
    public:
      static Ref<PhysicsWorld2D> GetPhysicsWorld(const glm::vec2& gravity);

    private:
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_ENGINE_HPP
