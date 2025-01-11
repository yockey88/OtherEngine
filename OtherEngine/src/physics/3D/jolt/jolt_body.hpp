/**
 * \file physics/3D/jolt/jolt_body.hpp
 **/
#ifndef OTHER_ENGINE_JOLT_BODY_HPP
#define OTHER_ENGINE_JOLT_BODY_HPP

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>

#include "physics/3D/physics_body.hpp"

namespace other {

  class JoltBody : public PhysicsBody {
   public:
    JoltBody(JPH::BodyInterface& body_interface, JPH::Body* body);
    virtual ~JoltBody() = default;

    void SetTransform(const Transform& transform) override;
    Transform GetTransform() const override;

    virtual glm::vec3 GetPosition() const override;
    virtual glm::quat GetOrientation() const override;
    virtual Transform InterpolateTransform(float alpha) override;

    virtual void AddCollider(Ref<PhysicsShape> shape) override;

   private:
    JPH::BodyInterface& body_interface;
    JPH::Body* body = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_BODY_HPP