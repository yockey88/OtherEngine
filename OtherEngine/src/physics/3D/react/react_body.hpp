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
    ReactBody(rp3d::RigidBody* body);
    virtual ~ReactBody() override;

    glm::vec3 GetPosition() const override;
    glm::quat GetOrientation() const override;
    Transform InterpolateTransform(const Transform& target, float alpha) const override;

    void AddCollider(Ref<PhysicsShape> shape) override;

   private:
    rp3d::RigidBody* body = nullptr;
    rp3d::Transform inter_transform;

    void OnBodyTypeChange(PhysicsBodyType type) override;
    void OnLayerChange(uint32_t layer) override {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_BODY_HPP