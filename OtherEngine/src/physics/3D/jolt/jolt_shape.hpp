/**
 * \file physics/3D/jolt/jolt_shape.hpp
 **/
#ifndef OTHER_ENGINE_JOLT_SHAPE_HPP
#define OTHER_ENGINE_JOLT_SHAPE_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include "physics/3D/physics_shape.hpp"

namespace other {

  class JoltShape : public PhysicsShape {
   public:
    virtual ~JoltShape() {}

   private:
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_SHAPE_HPP