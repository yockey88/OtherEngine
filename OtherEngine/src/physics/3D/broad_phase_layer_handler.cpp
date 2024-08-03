
/**
 * \file physics/3D/broad_phase_layer_handler.cpp
 **/
#include "physics/3D/broad_phase_layer_handler.hpp"

namespace other {

  JPH::uint BroadPhaseLayerHandler::GetNumBroadPhaseLayers() const {
    return 1;
  }
  
  JPH::BroadPhaseLayer BroadPhaseLayerHandler::GetBroadPhaseLayer(JPH::ObjectLayer layer) const {
    return broad_phase_layer;
  }
  
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
   const char* BroadPhaseLayerHandler::GetBroadPhaseLayerName(JPH::BroadBroadPhaseLayer layer) const {
     return "Debug Broad Phase Layer";
   }
#endif // !JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

} // namespace other
