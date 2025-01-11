/**
 * \file physics/3D/reacto/react_world.hpp
 **/
#ifndef OTHER_ENGINE_REACT_WORLD_HPP
#define OTHER_ENGINE_REACT_WORLD_HPP

#include <chrono>

#include <reactphysics3d/reactphysics3d.h>

#include "physics/3D/physics_body.hpp"
#include "physics/3D/physics_shape.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  using SteadyClock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<SteadyClock>;

  class ReactWorld : public PhysicsWorld {
   public:
    ReactWorld();
    virtual ~ReactWorld() override;

    virtual void ResetSimulation() override;
    virtual void Simulate(float ts) override;
    virtual Ref<PhysicsBody> CreateBody(Transform& initial_transform) override;

    virtual Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) override;
    virtual Ref<PhysicsShape> CreateSphereShape(float radius) override;
    virtual Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) override;
    virtual Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) override;
    virtual Ref<PhysicsShape> CreateConcaveMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) override;

    virtual void SetDebugRendering(bool debug) override;
    virtual void SubmitDebugRender(Ref<SceneRenderer> renderer) override;

   private:
    rp3d::PhysicsCommon physics_common;
    rp3d::PhysicsWorld* physics_world = nullptr;

    std::vector<glm::vec3> initial_positions;
    std::vector<glm::quat> initial_rotations;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_WORLD_HPP