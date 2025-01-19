/**
 * \file physics/collision_listener.cpp
 **/
#include "physics/collision_listener.hpp"

namespace other {

  void CollisionListener::HandleContact(UUID entity1, UUID entity2) {
    OE_ASSERT(scene_context != nullptr, "Scene context is null");
    auto [ent1, ent2] = scene_context->HandleContact(entity1, entity2);
    OnContact(ent1, ent2);
  }

}  // namespace other