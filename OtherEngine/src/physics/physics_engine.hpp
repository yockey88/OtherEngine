/**
 * \file physics/physics_engine.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_ENGINE_HPP
#define OTHER_ENGINE_PHYSICS_ENGINE_HPP

#include "core/config.hpp"
#include "core/ref.hpp"

#include "scene/scene.hpp"

#include "physics/2D/physics_world_2d.hpp"
#include "physics/3D/physics_world.hpp"

namespace other {

  class PhysicsEngine {
   public:
    static void Initialize(const ConfigTable& config);
    static void Shutdown();

    static void SetSceneContext(const Ref<Scene>& scene);

    static Ref<Scene> GetSceneContext();

    static Ref<PhysicsWorld2D> GetPhysicsWorld2D(const glm::vec2& gravity = glm::vec2(0.f, -9.81f));
    static Ref<PhysicsWorld> GetPhysicsWorld();

    static float GetInterpolationAlpha();

   private:
    static Ref<PhysicsWorld2D> physics_world_2d;
    static Ref<PhysicsWorld> physics_world;
    static Ref<Scene> scene_context;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PHYSICS_ENGINE_HPP
