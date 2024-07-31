
/**
 * \file physics/3D/physics_world.cpp
 **/
#include "physics/3D/physics_world.hpp"

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>


namespace other {

  constexpr static JPH::uint kMaxBodies = 1024;
  constexpr static JPH::uint kNumMutexes = 0;
  constexpr static JPH::uint kMaxPairs = 1024;
  constexpr static JPH::uint kMaxContactConstraints = 1024;

  PhysicsWorld::PhysicsWorld() {
    /// TODO: somehow choose the best amount of pre-allocated memory for the scene this world
    ///       is owned by
    temp_alloc = NewScope<JPH::TempAllocatorImpl>(10 * 1024 * 1024);
    thread_pool = NewScope<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs , JPH::cMaxPhysicsBarriers , 
                                                     std::thread::hardware_concurrency() - 1);

    system = NewScope<JPH::PhysicsSystem>();
    system->Init(kMaxBodies , kNumMutexes , kMaxPairs , kMaxContactConstraints ,
                 broad_phase_layer_handler , broad_phase_layer_filter , obj_layer_filter);

    activation_listener = NewScope<ActivationListener>();
    system->SetBodyActivationListener(activation_listener.get());
    
    // contact_listener = NewScope<ContactListener>();
    // system->SetContactListener(contact_listener.get());
  }

  PhysicsWorld::~PhysicsWorld() {
    system = nullptr;
    thread_pool = nullptr;
    temp_alloc = nullptr;
  }

  void PhysicsWorld::Simulate(float ts) {
    /// FIXME: do we do this each simulation frame
    system->OptimizeBroadPhase(); 

    const int32_t collision_steps = 16;
    system->Update(ts , collision_steps , temp_alloc.get() , thread_pool.get());
  }

  JPH::BodyInterface& PhysicsWorld::GetPhysicsBodies() {
    return system->GetBodyInterface();
  }

} // namespace other
