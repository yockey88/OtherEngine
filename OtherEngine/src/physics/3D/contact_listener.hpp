
/**
 * \file physics/3D/contact_listener.hpp
 **/
#ifndef OTHER_ENGINE_CONTACT_LISTENER_HPP
#define OTHER_ENGINE_CONTACT_LISTENER_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ContactListener.h>

namespace other {

  class ContactListener final : public JPH::ContactListener {
    public:
      virtual ~ContactListener() override {}
  
      JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg base_offset , 
                                            const JPH::CollideShapeResult& collision_result) override;
  
      void OnContactAdded(const JPH::Body& body1 , const JPH::Body& body2 , const JPH::ContactManifold& manifold , 
                          JPH::ContactSettings& settings) override;
  
      void OnContactPersisted(const JPH::Body& body1 , const JPH::Body& body2 , const JPH::ContactManifold& manifold , 
                              JPH::ContactSettings& settings) override;
  
      void OnContactRemoved(const JPH::SubShapeIDPair& sub_shape_pair) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_CONTACT_LISTENER_HPP
