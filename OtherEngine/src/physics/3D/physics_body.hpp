
/**
 * \file physics/3D/physics_body.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_BODY_HPP
#define OTHER_ENGINE_PHYSICS_BODY_HPP

#include <cstdint>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <glm/glm.hpp>

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

    virtual void SetTransform(const Transform& transform) = 0;
    virtual Transform GetTransform() const = 0;

    virtual glm::vec3 GetPosition() const = 0;
    virtual glm::quat GetOrientation() const = 0;
    virtual Transform InterpolateTransform(float alpha) = 0;

    virtual void AddCollider(Ref<PhysicsShape> shape) = 0;

    PhysicsBodyType GetType() const;
    uint32_t GetLayer() const;

   private:
    uint32_t object_layer = 0;
    PhysicsBodyType body_type = STATIC;

    virtual void OnBodyTypeChange(PhysicsBodyType type) {}
    virtual void OnLayerChange(uint32_t layer) {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_BODY_HPP
