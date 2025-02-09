/**
 * \file physics/3D/react/react_raycast_callback.cpp
 **/
#include "physics/3D/react/react_raycast_callback.hpp"

#include "scene/scene.hpp"

#include "physics/3D/react/react_world.hpp"

namespace other {

  ReactRaycastCallback::ReactRaycastCallback(ReactWorld* world, Scene* scene_ctx, float distance)
      : world(world), scene(scene_ctx), ray_cast_distance(distance) {
    OE_ASSERT(world != nullptr, "World is null!");
    OE_ASSERT(scene_ctx != nullptr, "Scene context is null!");
  }

  rp3d::decimal ReactRaycastCallback::notifyRaycastHit(const rp3d::RaycastInfo& info) {
    hit_occurred = true;
    PhysicsRaycastHit& hit = hits.emplace_back();

    UUID entity_id = *(UUID*)info.body->getUserData();
    auto* entity = scene->GetEntity(entity_id);
    OE_ASSERT(entity != nullptr, "Entity is null!");

    hit.entity_id = entity_id;
    hit.distance = info.hitFraction * ray_cast_distance;
    hit.point = glm::vec3(info.worldPoint.x, info.worldPoint.y, info.worldPoint.z);
    hit.normal = glm::vec3(info.worldNormal.x, info.worldNormal.y, info.worldNormal.z);

    return info.hitFraction;
  }

}  // namespace other