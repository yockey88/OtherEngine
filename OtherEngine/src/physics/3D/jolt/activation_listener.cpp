
/**
 * \file physics/3D/activation_listener.cpp
 **/
#include "physics/3D/jolt/activation_listener.hpp"

#include <Jolt/Physics/Body/BodyID.h>

#include "core/logger.hpp"

namespace other {

  void ActivationListener::OnBodyActivated(const JPH::BodyID& id, uint64_t body_user_data) {
    OE_DEBUG("Body activated : {}", id.GetIndex());
  }

  void ActivationListener::OnBodyDeactivated(const JPH::BodyID& id, uint64_t body_user_data) {
    OE_DEBUG("Body deactivated : {}", id.GetIndex());
  }

}  // namespace other
