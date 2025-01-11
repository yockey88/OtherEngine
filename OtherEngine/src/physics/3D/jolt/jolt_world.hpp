/**
 * \file physics/3D/jolt/jolt_world.hpp
 **/
#ifndef OTHER_ENGINE_JOLT_WORLD_HPP
#define OTHER_ENGINE_JOLT_WORLD_HPP

// clang-format off
#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyLockMulti.h>
#include <Jolt/Core/Profiler.h>
// clang-format on

#include "core/defines.hpp"

#include "physics/3D/jolt/activation_listener.hpp"
#include "physics/3D/jolt/broad_phase_filter.hpp"
#include "physics/3D/jolt/broad_phase_layer_handler.hpp"
#include "physics/3D/jolt/contact_listener.hpp"
#include "physics/3D/jolt/object_layer_filter.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  class JoltWorld : public PhysicsWorld {
   public:
    JoltWorld();
    virtual ~JoltWorld() override;

    void Simulate(float ts) override;
    Ref<PhysicsBody> CreateBody(Transform& initial_transform) override;

    Ref<PhysicsShape> CreateBoxShape(const glm::vec3& half_extents) override;
    Ref<PhysicsShape> CreateSphereShape(float radius) override;
    Ref<PhysicsShape> CreateCapsuleShape(float radius, float height) override;
    Ref<PhysicsShape> CreateConvexMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) override;
    Ref<PhysicsShape> CreateConcaveMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) override;

    void SetDebugRendering(bool debug) override {}
    void SubmitDebugRender(Ref<SceneRenderer> renderer) override {}

   private:
    Scope<JPH::TempAllocatorImpl> temp_alloc = nullptr;
    Scope<JPH::JobSystemThreadPool> thread_pool = nullptr;

    Scope<ActivationListener> activation_listener = nullptr;
    Scope<ContactListener> contact_listener = nullptr;

    BroadPhaseLayerHandler broad_phase_layer_handler;
    BroadPhaseLayerFilter broad_phase_layer_filter;
    ObjectLayerFilter obj_layer_filter;

    Scope<JPH::PhysicsSystem> system = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_JOLT_WORLD_HPP
