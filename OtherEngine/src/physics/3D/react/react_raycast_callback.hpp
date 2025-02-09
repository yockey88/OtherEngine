/**
 * \file physics/3D/react/react_raycast_callback.hpp
 **/
#ifndef OTHER_ENGINE_REACT_RAYCAST_CALLBACK_HPP
#define OTHER_ENGINE_REACT_RAYCAST_CALLBACK_HPP

#include <reactphysics3d/collision/RaycastInfo.h>

#include "physics/physics_defines.hpp"

namespace rp3d = reactphysics3d;

namespace other {

  class Scene;
  class ReactWorld;

  struct ReactRaycastCallback : public rp3d::RaycastCallback {
    ReactRaycastCallback(ReactWorld* world, Scene* scene_ctx, float distance);
    virtual ~ReactRaycastCallback() = default;

    rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) override;

    ReactWorld* world = nullptr;
    Scene* scene = nullptr;
    float ray_cast_distance = 0.0f;
    bool hit_occurred = false;

    std::vector<PhysicsRaycastHit> hits;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_REACT_RAYCAST_CALLBACK_HPP