/**
 * \file physics/physics_engine.cpp
 **/
#include "physics/phyics_engine.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Memory.h>
#include <Jolt/Core/Factory.h>

#include "application/app_state.hpp"

#include "physics/2D/physics_world_2d.hpp"

namespace other {
  
  Ref<Scene> PhysicsEngine::scene_context = nullptr;

  void PhysicsEngine::Initialize(const ConfigTable& config) {
    JPH::RegisterDefaultAllocator(); 

    /// register debug functions

    /// need to figure out what this is and if i can do this some other way
    JPH::Factory::sInstance = new JPH::Factory;

    /// Register engine types
    /// Register user types from config table

    JPH::RegisterTypes();
  }

  void PhysicsEngine::Shutdown() {
    JPH::UnregisterTypes();

    delete JPH::Factory::sInstance;
    JPH::Factory::sInstance = nullptr;
  }

  void PhysicsEngine::SetSceneContext(const Ref<Scene>& scene) {
    scene_context = scene;
  }
      
  Ref<Scene> PhysicsEngine::GetSceneContext() {
    return scene_context;
  }
      
  Ref<PhysicsWorld2D> PhysicsEngine::GetPhysicsWorld2D(const glm::vec2& gravity) {
    return Ref<PhysicsWorld2D>::Create(gravity);
  }

  Ref<PhysicsWorld> PhysicsEngine::GetPhysicsWorld() {
    return Ref<PhysicsWorld>::Create();
  }

} // namespace other
