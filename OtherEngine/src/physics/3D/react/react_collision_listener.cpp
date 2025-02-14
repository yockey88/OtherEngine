/**
 * \file physics/3D/reacto/react_collision_listener.cpp
 **/
#include "physics/3D/react/react_collision_listener.hpp"

#include "core/logger.hpp"

#include "scene/scene.hpp"

#include "physics/physics_defines.hpp"

namespace other {

  void ReactCollisionListener::Listener::onContact(const rp3d::CollisionCallback::CallbackData& data) {
    for (rp3d::uint i = 0; i < data.getNbContactPairs(); ++i) {
      rp3d::CollisionCallback::ContactPair pair = data.getContactPair(i);

      rp3d::Body* body1 = pair.getCollider1()->getBody();
      OE_ASSERT(body1 != nullptr, "Body 1 is null");

      rp3d::Collider* collider1 = pair.getCollider1();
      OE_ASSERT(collider1 != nullptr, "Collider 1 is null");

      UUID* entity1 = (UUID*)body1->getUserData();
      OE_ASSERT(entity1 != nullptr, "Entity 1 is null");

      rp3d::Body* body2 = pair.getCollider2()->getBody();
      OE_ASSERT(body2 != nullptr, "Body 2 is null");

      rp3d::Collider* collider2 = pair.getCollider2();
      OE_ASSERT(collider2 != nullptr, "Collider 2 is null");

      UUID* entity2 = (UUID*)body2->getUserData();
      OE_ASSERT(entity2 != nullptr, "Entity 2 is null");

      UUID& ent1 = *entity1;
      UUID& ent2 = *entity2;

      listener.BeginContact(ent1, ent2);

      for (rp3d::uint j = 0; j < pair.getNbContactPoints(); ++j) {
        rp3d::CollisionCallback::ContactPoint pa = pair.getContactPoint(j);
        rp3d::Vector3 world_loc1 = collider1->getLocalToWorldTransform() * pa.getLocalPointOnCollider1();
        rp3d::Vector3 world_loc2 = collider2->getLocalToWorldTransform() * pa.getLocalPointOnCollider2();

        CollisionPointData point1{ ent1, glm::vec3{ world_loc1.x, world_loc1.y, world_loc1.z } };
        CollisionPointData point2{ ent2, glm::vec3{ world_loc2.x, world_loc2.y, world_loc2.z } };
        listener.ContactPoint(&point1, &point2);
      }

      listener.EndContact(ent1, ent2);
    }
  }

}  // namespace other