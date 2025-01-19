/**
 * \file physics/3D/react_collision_listener.hpp
 **/
#ifndef OTHER_ENGINE_REACT_COLLISION_LISTENER_HPP
#define OTHER_ENGINE_REACT_COLLISION_LISTENER_HPP

#include <map>

#include <reactphysics3d/reactphysics3d.h>

#include "core/ref.hpp"

#include "physics/collision_listener.hpp"

namespace rp3d = reactphysics3d;

namespace other {

  class Scene;

  class ReactCollisionListener : public CollisionListener {
   public:
    ReactCollisionListener(Scene* scene)
        : CollisionListener(scene) {}

    struct Listener : public rp3d::EventListener, public CollisionCallback {
      void onContact(const rp3d::CollisionCallback::CallbackData& data) override;

      Listener(CollisionListener& listener)
          : CollisionListener::CollisionCallback(listener) {}
      virtual ~Listener() override = default;

    } listener{ *this };

   private:
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_COLLISION_LISTENER_HPP
