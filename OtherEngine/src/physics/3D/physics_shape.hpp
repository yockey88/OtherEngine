
/**
 * \file physics/3D/physics_shape.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_SHAPE_HPP
#define OTHER_ENGINE_PHYSICS_SHAPE_HPP

#include "core/ref_counted.hpp"

namespace other {

  class PhysicsShape : public RefCounted {
   public:
    virtual ~PhysicsShape() {}

   private:
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_SHAPE_HPP
