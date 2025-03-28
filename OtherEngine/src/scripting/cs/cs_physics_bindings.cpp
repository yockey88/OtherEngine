/**
 * \file scripting/cs/cs_physics_bindings.cpp
 **/
#include "scripting/cs/cs_physics_bindings.hpp"

#include "core/logger.hpp"

#include "physics/3D/physics_world.hpp"
#include "scripting/script_engine.hpp"

namespace other {
  namespace cs_script_bindings {

    bool NativeRaycast(Ray* ray, float* ray_length, uint64_t* hit_entity, glm::vec3* hit_normal, glm::vec3* hit_point, float* hit_distance) {
      OE_ASSERT(ray != nullptr, "Ray is null!");
      OE_ASSERT(hit_entity != nullptr, "Hit entity is null!");
      OE_ASSERT(hit_normal != nullptr, "Hit normal is null!");
      OE_ASSERT(hit_point != nullptr, "Hit point is null!");
      OE_ASSERT(hit_distance != nullptr, "Hit distance is null!");

      Ref<Scene> scene = ScriptEngine::GetSceneContext();
      if (scene == nullptr) {
        OE_ERROR("Attempting to raycast in without a scene context!");
        return false;
      }

      Ref<PhysicsWorld> physics_world = scene->GetPhysicsWorld();
      OE_ASSERT(physics_world != nullptr, "Physics world is null!");

      PhysicsRaycastHit hit;
      bool did_hit = physics_world->Raycast(hit, *ray, *ray_length);
      if (!did_hit) {
        return false;
      }

      *hit_entity = hit.entity_id.Get();
      *hit_normal = hit.normal;
      *hit_point = hit.point;
      *hit_distance = hit.distance;

      return true;
    }

  }  // namespace cs_script_bindings
}  // namespace other