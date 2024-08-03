
/**
 * \file physics/3D/object_layer_filter.cpp
 **/
#include "physics/3D/object_layer_filter.hpp"

namespace other {

  bool ObjectLayerFilter::ShouldCollide(JPH::ObjectLayer layer1 , JPH::ObjectLayer layer2) const {
    return true;
  }

} // namespace other
