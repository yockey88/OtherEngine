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
    JoltBody(JPH::BodyInterface& body_interface, Entity* entity);
    virtual ~JoltBody() = default;

    virtual glm::vec3 GetPosition() const override { return {}; }
    virtual glm::quat GetOrientation() const override { return {}; }
    virtual Transform InterpolateTransform(const Transform& target, float alpha) const override { return {}; }

    virtual void AddCollider(Ref<PhysicsShape> shape) override {}

   private:
    JPH::BodyInterface& body_interface;
    Entity* entity;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_BODY_HPP