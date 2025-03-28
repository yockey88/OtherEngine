
/**
 * \file physics/3D/contact_listener.cpp
 **/
#include "physics/3D/jolt/contact_listener.hpp"

#include <Jolt/Physics/Body/Body.h>

#include "core/logger.hpp"

namespace other {

  JPH::ValidateResult ContactListener::OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg base_offset, const JPH::CollideShapeResult& collision_result) {
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
  }

  void ContactListener::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) {
    OE_DEBUG("Contact added between bodies : {} and {}", body1.GetID().GetIndex(), body2.GetID().GetIndex());
  }

  void ContactListener::OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) {
    OE_DEBUG("Contact persisted between bodies : {} and {}", body1.GetID().GetIndex(), body2.GetID().GetIndex());
  }

  void ContactListener::OnContactRemoved(const JPH::SubShapeIDPair& sub_shape_pair) {
    OE_DEBUG("Contact removed between sub shapes : {} and {}", sub_shape_pair.GetSubShapeID1().GetValue(), sub_shape_pair.GetSubShapeID2().GetValue());
  }

}  // namespace other
