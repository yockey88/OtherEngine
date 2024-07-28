
/**
 * \file physics/3D/object_layer_filter.hpp
 **/
#ifndef OTHER_ENGINE_OBJECT_LAYER_FILTER_HPP
#define OTHER_ENGINE_OBJECT_LAYER_FILTER_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>

namespace other {

  class ObjectLayerFilter final : public JPH::ObjectLayerPairFilter {
    public:
      virtual ~ObjectLayerFilter() override {} 
  
      bool ShouldCollide(JPH::ObjectLayer layer1 , JPH::ObjectLayer layer2) const override;
  };

} // namespace other

#endif // !OTHER_ENGINE_OBJECT_LAYER_FILTER_HPP
