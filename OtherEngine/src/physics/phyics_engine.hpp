/**
 * \file physics/physics_engine.hpp
 **/
#ifndef OTHER_ENGINE_PHYSICS_ENGINE_HPP
#define OTHER_ENGINE_PHYSICS_ENGINE_HPP

#include "core/ref.hpp"

#include "scene/scene.hpp"

#include "physics/2D/physics_world_2d.hpp"

namespace other {

  class PhysicsEngine {
    public:
      static void SetSceneContext(const Ref<Scene>& scene);

      static Ref<Scene> GetSceneContext();

      static Ref<PhysicsWorld2D> GetPhysicsWorld(const glm::vec2& gravity);

    private:
      static Ref<Scene> scene_context;
  };

} // namespace other

#endif // !OTHER_ENGINE_PHYSICS_ENGINE_HPP
