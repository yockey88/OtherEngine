/**
 * \file physics/3D/jolt/jolt_world.hpp
 **/
#ifndef OTHER_ENGINE_JOLT_WORLD_HPP
#define OTHER_ENGINE_JOLT_WORLD_HPP

// clang-format off
#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSystem.h>
// clang-format on

#include "core/logger.hpp"

#include "ecs/components/physics_component.hpp"

#include "physics/3D/jolt/activation_listener.hpp"
#include "physics/3D/jolt/broad_phase_filter.hpp"
#include "physics/3D/jolt/broad_phase_layer_handler.hpp"
#include "physics/3D/jolt/contact_listener.hpp"
#include "physics/3D/jolt/jolt_body.hpp"
#include "physics/3D/jolt/jolt_shape.hpp"
#include "physics/3D/jolt/object_layer_filter.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  class JoltWorld : public PhysicsWorld {
   public:
    JoltWorld(Scene* scene_ctx);
    virtual ~JoltWorld() override;

    virtual void ResetSimulation(Scene* scene) override;
    void Simulate(float ts) override;
    void CreateBody(Entity& entity) override;
    void DestroyBody(Entity& entity) override;

    Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) override;
    Ref<PhysicsShape> CreateSphereShape(float radius) override;
    Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) override;
    Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) override;
    Ref<PhysicsShape> CreateConcaveMeshShape(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, uint32_t num_faces) override;
    Ref<PhysicsShape> CreateCompoundShape(const std::vector<Ref<PhysicsShape>>& shapes) override {
      OE_UNIMPLEMENTED_RETURN(nullptr);
    }

    void SetDebugRendering(bool debug) override {}
    void SubmitDebugRender(Ref<SceneRenderer> renderer) override {}

   private:
    JPH::TempAllocatorImpl* temp_alloc = nullptr;
    Scope<JPH::JobSystemThreadPool> thread_pool = nullptr;
    Scope<JPH::PhysicsSystem> system = nullptr;
    Scope<ActivationListener> activation_listener = nullptr;
    Scope<ContactListener> contact_listener = nullptr;

    BroadPhaseLayerHandler broad_phase_layer_handler;
    BroadPhaseLayerFilter broad_phase_layer_filter;
    ObjectLayerFilter obj_layer_filter;

    void CreateColliders(Collider& collider, Transform& transform);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_WORLD_HPP
