/**
 * \file physics/3D/jolt/jolt_world.cpp
 **/
#include "physics/3D/jolt/jolt_world.hpp"

#include "physics/3D/jolt/jolt_body.hpp"

namespace other {

  constexpr static JPH::uint kMaxBodies = 1024;
  constexpr static JPH::uint kNumMutexes = 0;
  constexpr static JPH::uint kMaxPairs = 1024;
  constexpr static JPH::uint kMaxContactConstraints = 1024;
  constexpr static JPH::uint kCollisionSteps = 1;

  static void JoltTrace(const char* in_msg, ...) {
    char buffer[1024];

    va_list args;
    va_start(args, in_msg);
    vprintf(buffer, args);
    va_end(args);

    std::string in_msg_str = in_msg;
    std::string msg = buffer;
    OE_DEBUG("Jolt > {}", in_msg_str, msg);
  }

  JoltWorld::JoltWorld() {
    JPH::Trace = JoltTrace;

    /// TODO: somehow choose the best amount of pre-allocated memory for the scene this world
    ///       is owned by ??? is there something we can calculate to know the optimal amount of memory per scene?
    ///       would this be part of mesh baking and scene compilation?
    temp_alloc = NewScope<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    thread_pool = NewScope<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    system = NewScope<JPH::PhysicsSystem>();
    system->Init(kMaxBodies, kNumMutexes, kMaxPairs, kMaxContactConstraints, broad_phase_layer_handler, broad_phase_layer_filter, obj_layer_filter);

    activation_listener = NewScope<ActivationListener>();
    system->SetBodyActivationListener(activation_listener.get());

    contact_listener = NewScope<ContactListener>();
    system->SetContactListener(contact_listener.get());

    system->OptimizeBroadPhase();
  }

  JoltWorld::~JoltWorld() {
    system = nullptr;
    thread_pool = nullptr;
    temp_alloc = nullptr;
  }

  void JoltWorld::Simulate(float ts) {
    JPH_PROFILE("JoltWorld::Simulate");
    system->Update(ts, kCollisionSteps, temp_alloc.get(), thread_pool.get());
  }

  Ref<PhysicsBody> JoltWorld::CreateBody(Transform& initial_transform) {
    JPH::BodyCreationSettings settings;

    return NewRef<JoltBody>(system->GetBodyInterface(), nullptr);
  }

  Ref<PhysicsShape> JoltWorld::CreateBoxShape(const glm::vec3& half_extents) {
    return nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateSphereShape(float radius) {
    return nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateCapsuleShape(float radius, float height) {
    return nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateConvexMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) {
    return nullptr;
  }

  Ref<PhysicsShape> JoltWorld::CreateConcaveMeshShape(const std::vector<glm::vec3>& vertices, const std::vector<uint32_t>& indices, uint32_t num_faces) {
    return nullptr;
  }

}  // namespace other
