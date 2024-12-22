/**
 * \file physics/3D/reacto/react_world.hpp
 **/
#ifndef OTHER_ENGINE_REACT_WORLD_HPP
#define OTHER_ENGINE_REACT_WORLD_HPP

#include <reactphysics3d/reactphysics3d.h>

#include "physics/3D/physics_body.hpp"
#include "physics/3D/physics_shape.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  class ReactWorld : public PhysicsWorld {
   public:
    ReactWorld();
    virtual ~ReactWorld() override;

    virtual void Simulate(float ts) override;
    virtual Ref<PhysicsBody> CreateBody(const Transform& initial_transform) override;

    virtual Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) override;
    virtual Ref<PhysicsShape> CreateSphereShape(float radius) override;
    virtual Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) override;
    virtual Ref<PhysicsShape> CreateCylinderShape(float radius, float height) override;
    virtual Ref<PhysicsShape> CreateConeShape(float radius, float height) override;
    virtual Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) override;

    virtual void SetSceneContext(const Ref<Scene>& scene) override;

   private:
    rp3d::PhysicsCommon physics_common;
    rp3d::PhysicsWorld* physics_world = nullptr;

    rp3d::decimal accumulator = 0.f;
    float alpha = 0.f;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_WORLD_HPP