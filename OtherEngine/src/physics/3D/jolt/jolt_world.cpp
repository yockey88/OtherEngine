/**
 * \file physics/3D/jolt/jolt_world.cpp
 **/
#include "physics/3D/jolt/jolt_world.hpp"

namespace other {

  constexpr static JPH::uint kMaxBodies = 1024;
  constexpr static JPH::uint kNumMutexes = 0;
  constexpr static JPH::uint kMaxPairs = 1024;
  constexpr static JPH::uint kMaxContactConstraints = 1024;

  JoltWorld::JoltWorld() {
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

}  // namespace other