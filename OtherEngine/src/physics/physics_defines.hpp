/**
 * \file physics/physics_defines.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_DEFINES_HPP
#define OTHER_ENGINE_PHYSICS_DEFINES_HPP

#include <cstdint>

namespace other {

  enum PhysicsBodyType : uint32_t {
    STATIC = 0, 
    KINEMATIC ,
    DYNAMIC ,

    NUM_PHYSICS_BODIES ,
    INVALID_PHYSICS_BODY = NUM_PHYSICS_BODIES
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_DEFINES_HPP