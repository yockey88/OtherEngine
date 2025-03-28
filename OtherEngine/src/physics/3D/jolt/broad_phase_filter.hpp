
/**
 * \file physics/3D/broad_phase_filter.hpp
 **/
#ifndef OTHER_ENGINE_BROAD_PHASE_FILTER_HPP
#define OTHER_ENGINE_BROAD_PHASE_FILTER_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace other {

  class BroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter {
    public:
      virtual ~BroadPhaseLayerFilter() override {}
  
      bool ShouldCollide(JPH::ObjectLayer layer1 , JPH::BroadPhaseLayer layer2) const override;
  };


} // namespace other

#endif // !OTHER_ENGINE_BROAD_PHASE_FILTER_HPP
