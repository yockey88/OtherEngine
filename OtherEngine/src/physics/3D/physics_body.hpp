
/**
 * \file physics/3D/physics_body.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_BODY_HPP
#define OTHER_ENGINE_PHYSICS_BODY_HPP

#include <cstdint>

#include <glm/glm.hpp>

#include "core/logger.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "ecs/components/transform.hpp"

#include "physics/3D/physics_shape.hpp"
#include "physics/physics_defines.hpp"

namespace other {

  class Entity;

  class PhysicsBody : public RefCounted {
   public:
    PhysicsBody() {}
    virtual ~PhysicsBody() = default;

    void SetType(PhysicsBodyType type);
    void SetLayer(uint32_t layer);
    void SetNativeBody(void* body);
    void SetEntityID(const UUID& id);

    virtual void OnSetNativeBody(void* body) = 0;

    virtual void SetTransform(const Transform& transform) = 0;
    virtual Transform GetTransform() const = 0;

    virtual glm::vec3 GetPosition() const = 0;
    virtual glm::quat GetOrientation() const = 0;
    virtual Transform InterpolateTransform(float alpha) = 0;

    virtual void SetVelocity(const glm::vec3& velocity) = 0;
    virtual glm::vec3 GetVelocity() const = 0;

    virtual void SetAngularVelocity(const glm::vec3& velocity) = 0;
    virtual glm::vec3 GetAngularVelocity() const = 0;

    virtual void SetMass(float mass) = 0;
    virtual float GetMass() const = 0;

    virtual void SetLinearDrag(float drag) = 0;
    virtual float GetLinearDrag() const = 0;

    virtual void SetAngularDrag(float drag) = 0;
    virtual float GetAngularDrag() const = 0;

    virtual void SetGravityScale(float scale) = 0;
    virtual float GetGravityScale() const = 0;

    virtual void SetFixedRotation(bool fixed) = 0;
    virtual bool GetFixedRotation() const = 0;

    virtual void SetBullet(bool bullet) = 0;
    virtual bool GetBullet() const = 0;

    virtual void SetTrigger(bool trigger) = 0;
    virtual bool IsTrigger() const = 0;

    // virtual void SetCollisionDetection(CollisionDetectionType type) = 0;
    // virtual void ApplyForce(const glm::vec3& force) = 0;
    // virtual void ApplyImpulse(const glm::vec3& impulse) = 0;
    // virtual void ApplyTorque(const glm::vec3& torque) = 0;
    // virtual void ApplyTorqueImpulse(const glm::vec3& torque) = 0;

    void AddCollider(Ref<PhysicsShape> shape);
    void RemoveCollider(Ref<PhysicsShape> shape);

    virtual void OnAddCollider(Ref<PhysicsShape> shape) = 0;
    virtual void OnRemoveCollider(Ref<PhysicsShape> shape) = 0;

    UUID GetEntityID() const;
    PhysicsBodyType GetType() const;
    uint32_t GetLayer() const;

    template <typename NB>
    NB* GetNativeBody() {
      OE_ASSERT(native_body != nullptr, "Native body is null!");
      return static_cast<NB*>(native_body);
    }

   protected:
    UUID entity_id;

   private:
    void* native_body = nullptr;
    uint32_t object_layer = 0;
    PhysicsBodyType body_type = STATIC;

    std::vector<Ref<PhysicsShape>> collider_shapes = {};
    Ref<PhysicsShape> collider_shape = nullptr;

    virtual void OnBodyTypeChange(PhysicsBodyType type) {}
    virtual void OnLayerChange(uint32_t layer) {}
    virtual void OnSetEntity() {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_BODY_HPP
