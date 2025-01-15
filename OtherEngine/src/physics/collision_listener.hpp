/**
 * \file physics/collision_listener.hpp
 **/
#ifndef OTHER_ENGINE_COLLISION_LISTENER_HPP
#define OTHER_ENGINE_COLLISION_LISTENER_HPP

#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

namespace other {

  class CollisionListener : public RefCounted {
   public:
    virtual void OnContact(UUID entity1, UUID entity2) = 0;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_COLLISION_LISTENER_HPP