/**
 * \file physics/3D/react/react_body.hpp
 **/
#ifndef OTHER_ENGINE_REACT_BODY_HPP
#define OTHER_ENGINE_REACT_BODY_HPP

#include <cstdint>

#include <reactphysics3d/mathematics/Transform.h>
#include <reactphysics3d/reactphysics3d.h>

#include "ecs/components/transform.hpp"

#include "physics/3D/physics_body.hpp"

namespace other {

  class ReactBody : public PhysicsBody {
   public:
    ReactBody(rp3d::RigidBody* body, rp3d::PhysicsWorld* world);
    virtual ~ReactBody() override;

    virtual void OnSetNativeBody(void* body) override;

    void SetTransform(const Transform& transform) override;
    Transform GetTransform() const override;

    glm::vec3 GetPosition() const override;
    glm::quat GetOrientation() const override;
    Transform InterpolateTransform(float alpha) override;

    void SetVelocity(const glm::vec3& velocity) override;
    glm::vec3 GetVelocity() const override;

    void SetAngularVelocity(const glm::vec3& velocity) override;
    glm::vec3 GetAngularVelocity() const override;

    void SetMass(float mass) override;
    float GetMass() const override;

    void SetLinearDrag(float drag) override;
    float GetLinearDrag() const override;

    void SetAngularDrag(float drag) override;
    float GetAngularDrag() const override;

    void SetGravityScale(float scale) override;
    float GetGravityScale() const override;

    void SetFixedRotation(bool fixed) override;
    bool GetFixedRotation() const override;

    void SetBullet(bool bullet) override;
    bool GetBullet() const override;

    void SetTrigger(bool trigger) override;
    bool IsTrigger() const override;

    void OnAddCollider(Ref<PhysicsShape> shape) override;
    void OnRemoveCollider(Ref<PhysicsShape> shape) override;

   private:
    rp3d::RigidBody* body = nullptr;
    rp3d::Collider* collider = nullptr;

    rp3d::PhysicsWorld* world = nullptr;
    rp3d::Transform inter_transform;

    void OnBodyTypeChange(PhysicsBodyType type) override;
    void OnLayerChange(uint32_t layer) override {}
    void OnSetEntity() override;

    Transform ExtractTransform(const rp3d::Transform& physics_transform) const;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_BODY_HPP