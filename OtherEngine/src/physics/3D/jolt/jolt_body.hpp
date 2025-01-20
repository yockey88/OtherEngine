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

    void OnSetNativeBody(void* body) override;

    void SetTransform(const Transform& transform) override;
    Transform GetTransform() const override;

    glm::vec3 GetPosition() const override;
    glm::quat GetOrientation() const override;
    Transform InterpolateTransform(float alpha) override;

    void SetVelocity(const glm::vec3& velocity) override {}
    glm::vec3 GetVelocity() const override { return {}; }

    void SetAngularVelocity(const glm::vec3& velocity) override {}
    glm::vec3 GetAngularVelocity() const override { return {}; }

    void SetMass(float mass) override {}
    float GetMass() const override { return 0.f; }

    void SetLinearDrag(float drag) override {}
    float GetLinearDrag() const override { return 0.f; }

    void SetAngularDrag(float drag) override {}
    float GetAngularDrag() const override { return 0.f; }

    void SetGravityScale(float scale) override {}
    float GetGravityScale() const override { return 0.f; }

    void SetFixedRotation(bool fixed) override {}
    bool GetFixedRotation() const override { return false; }

    void SetBullet(bool bullet) override {}
    bool GetBullet() const override { return false; }

    void SetTrigger(bool trigger) override {}
    bool IsTrigger() const override { return false; }

    void OnAddCollider(Ref<PhysicsShape> shape) override;
    void OnRemoveCollider(Ref<PhysicsShape> shape) override;

   private:
    JPH::BodyInterface& body_interface;
    JPH::Body* body = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_BODY_HPP