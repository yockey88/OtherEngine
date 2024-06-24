/**
 * \file physics/physics_engine.cpp
 **/
#include "physics/phyics_engine.hpp"

#include "physics/2D/physics_world_2d.hpp"

namespace other {
  
  Ref<Scene> PhysicsEngine::scene_context = nullptr;

  void PhysicsEngine::SetSceneContext(const Ref<Scene>& scene) {
    scene_context = scene;
  }
      
  Ref<Scene> PhysicsEngine::GetSceneContext() {
    return scene_context;
  }
      
  Ref<PhysicsWorld2D> PhysicsEngine::GetPhysicsWorld(const glm::vec2& gravity) {
    return Ref<PhysicsWorld2D>::Create(gravity);
  }


} // namespace other
