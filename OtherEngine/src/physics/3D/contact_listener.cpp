
/**
 * \file physics/3D/contact_listener.cpp
 **/
#include "physics/3D/contact_listener.hpp"

namespace other {


  JPH::ValidateResult ContactListener::OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg base_offset , 
                                        const JPH::CollideShapeResult& collision_result) {
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
  }
  
  void ContactListener::OnContactAdded(const JPH::Body& body1 , const JPH::Body& body2 , const JPH::ContactManifold& manifold , 
                      JPH::ContactSettings& settings) {}
  
  void ContactListener::OnContactPersisted(const JPH::Body& body1 , const JPH::Body& body2 , const JPH::ContactManifold& manifold , 
                          JPH::ContactSettings& settings) {}
  
  void ContactListener::OnContactRemoved(const JPH::SubShapeIDPair& sub_shape_pair) {}

} // namespace other
