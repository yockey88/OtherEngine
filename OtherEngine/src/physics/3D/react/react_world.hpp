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
#include "physics/3D/react/react_collision_listener.hpp"

namespace other {

  using SteadyClock = std::chrono::steady_clock;
  using TimePoint = std::chrono::time_point<SteadyClock>;

  class ReactWorld : public PhysicsWorld {
   public:
    ReactWorld(Scene* scene_ctx);
    virtual ~ReactWorld() override;

    void ResetSimulation(Scene* scene) override;
    void Simulate(float ts) override;
    void CreateBody(Entity& entity) override;
    void DestroyBody(Entity& entity) override;

    Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) override;
    Ref<PhysicsShape> CreateSphereShape(float radius) override;
    Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) override;
    Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) override;
    Ref<PhysicsShape> CreateConcaveMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) override;
    Ref<PhysicsShape> CreateCompoundShape(const std::vector<Ref<PhysicsShape>>& shapes) override;
    Ref<PhysicsShape> CreateTerrainShape(const Terrain& terrain) override;

    void SetDebugRendering(bool debug) override;
    void SubmitDebugRender(Ref<SceneRenderer> renderer) override;

   private:
    rp3d::PhysicsCommon physics_common;

    rp3d::PhysicsWorld::WorldSettings settings;
    rp3d::PhysicsWorld* physics_world = nullptr;

    Ref<ReactCollisionListener> collision_listener = nullptr;

    void RegisterCallbacks() override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_WORLD_HPP