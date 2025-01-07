
/**
 * \file physics/3D/jolt/broad_phase_filter.cpp
 **/
#include "physics/3D/jolt/broad_phase_filter.hpp"

namespace other {

  bool BroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const {
    return true;
  }

}  // namespace other
