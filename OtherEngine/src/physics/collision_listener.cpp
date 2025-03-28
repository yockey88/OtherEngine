/**
 * \file physics/collision_listener.cpp
 **/
#include "physics/collision_listener.hpp"

#include "ecs/entity.hpp"
#include "scene/scene.hpp"

namespace other {

  void CollisionListener::BeginContact(UUID entity1, UUID entity2) {
    OE_ASSERT(scene_context != nullptr, "Scene context is null");
    auto [ent1, ent2] = scene_context->BeginContact(entity1, entity2);
    BeginContact(ent1, ent2);
  }

  void CollisionListener::ContactPoint(CollisionPointData* point1, CollisionPointData* point2) {
    OE_ASSERT(scene_context != nullptr, "Scene context is null");
    auto [ent1, ent2] = scene_context->ContactPoint(point1, point2);
    ContactPoint(ent1, ent2);
  }

  void CollisionListener::EndContact(UUID entity1, UUID entity2) {
    OE_ASSERT(scene_context != nullptr, "Scene context is null");
    auto [ent1, ent2] = scene_context->EndContact(entity1, entity2);
    EndContact(ent1, ent2);
  }

}  // namespace other