/**
 * \file physics/physics_engine.cpp
 **/
#include "physics/physics_engine.hpp"

// #include <Jolt/Core/Factory.h>
// #include <Jolt/Core/Memory.h>
// #include <Jolt/Jolt.h>
// #include <Jolt/RegisterTypes.h>

#include "application/app_state.hpp"

#include "physics/2D/physics_world_2d.hpp"
#include "physics/3D/react/react_world.hpp"

namespace other {

  Ref<PhysicsWorld2D> PhysicsEngine::physics_world_2d = nullptr;
  Ref<PhysicsWorld> PhysicsEngine::physics_world = nullptr;
  Ref<Scene> PhysicsEngine::scene_context = nullptr;

  void PhysicsEngine::Initialize(const ConfigTable& config) {
    // JPH::RegisterDefaultAllocator();

    // /// register debug functions

    // /// need to figure out what this is and if i can do this some other way
    // JPH::Factory::sInstance = new JPH::Factory;

    // /// Register engine types
    // /// Register user types from config table

    // JPH::RegisterTypes();
  }

  void PhysicsEngine::Shutdown() {
    // JPH::UnregisterTypes();

    // delete JPH::Factory::sInstance;
    // JPH::Factory::sInstance = nullptr;
  }

  void PhysicsEngine::SetSceneContext(const Ref<Scene>& scene) {
    scene_context = scene;
    if (scene != nullptr) {
      // scene->SetPhysicsSpec(PhysicsSpec());
    }

    Ref<PhysicsWorld> world = GetPhysicsWorld();
    OE_ASSERT(world != nullptr, "Failed to create 3D physics world");

    world->SetSceneContext(scene);
  }

  Ref<Scene> PhysicsEngine::GetSceneContext() {
    return scene_context;
  }

  Ref<PhysicsWorld2D> PhysicsEngine::GetPhysicsWorld2D(const glm::vec2& gravity) {
    if (physics_world_2d == nullptr) {
      physics_world_2d = NewRef<PhysicsWorld2D>(gravity);
    }
    return physics_world_2d;
  }

  Ref<PhysicsWorld> PhysicsEngine::GetPhysicsWorld() {
    if (physics_world == nullptr) {
      physics_world = NewRef<ReactWorld>();
    }
    return physics_world;
  }

  float PhysicsEngine::GetInterpolationAlpha() {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    return physics_world->alpha;
  }

}  // namespace other
