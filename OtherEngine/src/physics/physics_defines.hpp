/**
 * \file physics/physics_defines.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_DEFINES_HPP
#define OTHER_ENGINE_PHYSICS_DEFINES_HPP

#include <cstdint>

#include "core/defines.hpp"

namespace other {
  
  enum PhysicsType {
    PHYSICS_2D , 
    PHYSICS_3D ,

    NUM_PHYSICS_TYPES ,
    INVALID_PHYSICS_TYPE = NUM_PHYSICS_TYPES
  };

  enum PhysicsBodyType : uint32_t {
    STATIC = 0, 
    KINEMATIC ,
    DYNAMIC ,

    NUM_PHYSICS_BODIES ,
    INVALID_PHYSICS_BODY = NUM_PHYSICS_BODIES
  };

  enum MeshCookingResult {
    NUM_COOKING_RESULTS , 
    INVALID_COOKING_RESULT = NUM_COOKING_RESULTS ,
  };

  enum ForceMode {
    FORCE = 0 , 
    IMPULSE , 
    VELOCITY_CHANGE , 
    ACCELERATION ,

    NUM_FORCE_MODES , 
    INVALID_FORCE_MODE = NUM_FORCE_MODES ,
  };

  enum class ActorAxis : uint32_t {
    NO_AXIS = 0 ,

    TRANSLATION_X = bit(1) ,
    TRANSLATION_Y = bit(2) , 
    TRANSLATION_Z = bit(3) ,
    TRANSLATION = TRANSLATION_X | TRANSLATION_Y | TRANSLATION_Z ,
    
    ROTATION_X = bit(4) , 
    ROTATION_Y = bit(5) , 
    ROTATION_Z = bit(6) , 
    ROTATION = ROTATION_X | ROTATION_Y | ROTATION_Z ,

    NUM_ACTOR_AXES , 
    INVALID_ACTOR_AXIS = NUM_ACTOR_AXES ,
  };

  enum CollisionDetectionType {
    DISCRETE_COLLISION = 0 , 
    CONTINUOUS_COLLISION ,

    NUM_COLLISION_DETECTIONS , 
    INVALID_COLLISION_DETECTION = NUM_COLLISION_DETECTIONS ,
  };

  enum CollisionFlags : uint8_t {
    NO_COLLISION = 0 , 
    SIDE_COLLISION = bit(1) ,
    ABOVE_COLLISION = bit(2) , 
    BELOW_COLLISION = bit(3) ,

    NUM_COLLISON_FLAGS , 
    INVALID_COLLISON_FLAG = NUM_COLLISON_FLAGS ,
  };

  enum FallOffMode {
    CONSTANT_FALLOFF = 0 , 
    LINEAR_FALLOFF ,

    NUM_FALL_OFF_MODES , 
    INVALID_FALL_OFF_MODE = NUM_COOKING_RESULTS ,
  };

  struct CollisionMaterial {
    float friction = 0.5f;
    float restitution = 0.15f;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_DEFINES_HPP
