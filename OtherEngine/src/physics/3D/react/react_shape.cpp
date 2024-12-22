/**
 * \file physics/3D/react/react_shape.pp
 **/
#include "physics/3D/react/react_shape.hpp"

namespace other {

  ReactShape::ReactShape(rp3d::CollisionShape* shape)
      : shape(shape) {
  }

  ReactShape::~ReactShape() {}

}  // namespace other