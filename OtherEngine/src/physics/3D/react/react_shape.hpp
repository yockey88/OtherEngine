/**
 * \file physics/3D/react/react_shape.hpp
 **/
#ifndef OTHER_ENGINE_REACT_SHAPE_HPP
#define OTHER_ENGINE_REACT_SHAPE_HPP

#include <reactphysics3d/reactphysics3d.h>

#include "physics/3D/physics_shape.hpp"

namespace other {

  class ReactShape : public PhysicsShape {
   public:
    ReactShape(rp3d::CollisionShape* shape);
    virtual ~ReactShape() override;

    rp3d::CollisionShape* shape = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_SHAPE_HPP