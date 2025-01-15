/**
 * \file physics/3D/reacto/react_collision_listener.cpp
 **/
#include "physics/3D/react/react_collision_listener.hpp"

#include "core/logger.hpp"

namespace other {

  void ReactCollisionListener::Listener::onContact(const rp3d::CollisionCallback::CallbackData& data) {
    for (rp3d::uint i = 0; i < data.getNbContactPairs(); ++i) {
      rp3d::CollisionCallback::ContactPair pair = data.getContactPair(i);

      rp3d::Body* body1 = pair.getCollider1()->getBody();
      rp3d::Collider* collider1 = pair.getCollider1();

      rp3d::Body* body2 = pair.getCollider2()->getBody();
      rp3d::Collider* collider2 = pair.getCollider2();

      for (rp3d::uint j = 0; j < pair.getNbContactPoints(); ++j) {
        rp3d::CollisionCallback::ContactPoint pa = pair.getContactPoint(j);

        rp3d::Vector3 world_loc1 = collider1->getLocalToWorldTransform() * pa.getLocalPointOnCollider1();
        rp3d::Vector3 world_loc2 = collider2->getLocalToWorldTransform() * pa.getLocalPointOnCollider2();

        // UUID entity1 = body_map[collider1];
        // UUID entity2 = body_map[collider2];

        // OnContact(entity1, entity2);
      }
    }
  }

}  // namespace other