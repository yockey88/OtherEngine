/**
 * \file physics/collision_listener.hpp
 **/
#ifndef OTHER_ENGINE_COLLISION_LISTENER_HPP
#define OTHER_ENGINE_COLLISION_LISTENER_HPP

#include <glm/fwd.hpp>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  class Entity;
  class Scene;

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

    void BeginContact(UUID entity1, UUID entity2);
    void ContactPoint(CollisionPointData* point1, CollisionPointData* point2);
    void EndContact(UUID entity1, UUID entity2);

   protected:
    Scene* scene_context = nullptr;

    virtual void BeginContact(Entity* entity1, Entity* entity2) {}
    virtual void ContactPoint(Entity* entity1, Entity* entity2) {}
    virtual void EndContact(Entity* entity1, Entity* entity2) {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COLLISION_LISTENER_HPP