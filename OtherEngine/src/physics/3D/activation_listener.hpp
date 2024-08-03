
/**
 * \file physics/3D/activation_listener.hpp
 **/
#ifndef OTHER_ENGINE_ACTIVATION_LISTENER_HPP
#define OTHER_ENGINE_ACTIVATION_LISTENER_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

namespace other {

  class ActivationListener final : public JPH::BodyActivationListener {
    public:
      virtual ~ActivationListener() override {}
  
      void OnBodyActivated(const JPH::BodyID& id , uint64_t body_user_data) override;
  
      void OnBodyDeactivated(const JPH::BodyID& id , uint64_t body_user_data) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_ACTIVATION_LISTENER_HPP
