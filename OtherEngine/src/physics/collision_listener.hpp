/**
 * \file physics/collision_listener.hpp
 **/
#ifndef OTHER_ENGINE_COLLISION_LISTENER_HPP
#define OTHER_ENGINE_COLLISION_LISTENER_HPP

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "scene/scene.hpp"

namespace other {

  class CollisionListener : public RefCounted {
   public:
    class CollisionCallback {
     public:
      CollisionCallback(CollisionListener& listener)
          : listener(listener) {}

     protected:
      CollisionListener& listener;
    };

    CollisionListener(Scene* scene)
        : scene_context(scene) {}

    void HandleContact(UUID entity1, UUID entity2);
    virtual void OnContact(Entity* entity1, Entity* entity2) {}

   protected:
    Ref<Scene> scene_context = nullptr;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COLLISION_LISTENER_HPP