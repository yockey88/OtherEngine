/**
 * \file physics/3D/react_collision_listener.hpp
 **/
#ifndef OTHER_ENGINE_REACT_COLLISION_LISTENER_HPP
#define OTHER_ENGINE_REACT_COLLISION_LISTENER_HPP

#include <map>

#include <reactphysics3d/reactphysics3d.h>

#include "physics/collision_listener.hpp"

namespace rp3d = reactphysics3d;

namespace other {

  class ReactCollisionListener : public CollisionListener {
   public:
    struct Listener : public rp3d::EventListener {
      void onContact(const rp3d::CollisionCallback::CallbackData& data) override;
    } listener;

    void OnContact(UUID entity1, UUID entity2) override {}

    void RegisterBody(rp3d::Collider* collider, UUID entity_id) {}

   private:
    std::map<rp3d::Collider*, UUID> body_map;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_COLLISION_LISTENER_HPP
