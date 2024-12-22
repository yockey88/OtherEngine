
/**
 * \file physics/3D/broad_phase_layer_handler.hpp
 **/
#ifndef OTHER_ENGINE_BROAD_PHASE_LAYER_HANDLER_HPP
#define OTHER_ENGINE_BROAD_PHASE_LAYER_HANDLER_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

namespace other {

  class BroadPhaseLayerHandler : public JPH::BroadPhaseLayerInterface {
    public:
      BroadPhaseLayerHandler() {}
      virtual ~BroadPhaseLayerHandler() override {}
  
      JPH::uint GetNumBroadPhaseLayers() const override;
      JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override;
  
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
      const char* GetBroadPhaseLayerName(JPH::BroadBroadPhaseLayer layer) const override;
#endif // !JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
  
    private:
      JPH::BroadPhaseLayer broad_phase_layer;
  };

} // namespace other

#endif // !OTHER_ENGINE_BROAD_PHASE_LAYER_HANDLER_HPP
