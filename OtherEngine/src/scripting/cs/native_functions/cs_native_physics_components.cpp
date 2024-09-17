
/**
 * \file scripting/cs/native_functions/cs_native_physics_components.cpp
 **/
#include "scripting/cs/native_functions/cs_native_physics_components.hpp"

#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/collider_2d.hpp"

#include "scripting/cs/native_functions/cs_native_helpers.hpp"

namespace other {
namespace cs_script_bindings {

  uint32_t NativeGetPhysicsBodyType(uint64_t id) {
    return NativeDoThing<uint32_t>(id , [](Entity* ent) -> uint32_t {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving rigid body type from entity without a rigid body : {}" , ent->Name());
        return 0;
      }

      return ent->ReadComponent<RigidBody2D>().type;
    });
  }
   
  void NativeSetPhysicsBodyType(uint64_t id, uint32_t type) {
    NativeDoThing<void>(id , [&type](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving rigid body type from entity without a rigid body : {}" , ent->Name());
        return;
      }

      ent->GetComponent<RigidBody2D>().type = static_cast<PhysicsBodyType>(type);
    });
  }
  
  float NativeGetMass(uint64_t id) {
    return NativeDoThing<float>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving mass from entity without a rigid body : {}" , entity->Name());
        return 0.f;
      }

      return entity->ReadComponent<RigidBody2D>().mass;
    });
  }
  
  void NativeSetMass(uint64_t id , float* mass) {
    NativeDoThing<void>(id , [&mass](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving mass from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().mass = *mass;
    });
  }

  float NativeGetLinearDrag(uint64_t id) {
    return NativeDoThing<float>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving linear drag from entity without a rigid body : {}" , entity->Name());
        return 0.f;
      }

      return entity->ReadComponent<RigidBody2D>().linear_drag;
    });
  }

  float NativeGetAngularDrag(uint64_t id) {
    return NativeDoThing<float>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving angular drag from entity without a rigid body : {}" , entity->Name());
        return 0.f;
      }

      return entity->ReadComponent<RigidBody2D>().angular_drag;
    });
  }

  float NativeGetGravityScale(uint64_t id) {
    return NativeDoThing<float>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving gravity scale from entity without a rigid body : {}" , entity->Name());
        return 0.f;
      }

      return entity->ReadComponent<RigidBody2D>().gravity_scale;
    });
  }

  bool NativeGetFixedRotation(uint64_t id) {
    return NativeDoThing<bool>(id , [](Entity* entity) -> bool {
      if (!entity->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving fixed rotation from entity without a rigid body : {}" , entity->Name());
        return false;
      }

      return entity->ReadComponent<RigidBody2D>().fixed_rotation;
    });
  }

  bool NativeGetBullet(uint64_t id) {
    return NativeDoThing<bool>(id , [](Entity* entity) -> bool {
      if (!entity->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving bullet from entity without a rigid body : {}" , entity->Name());
        return false;
      }

      return entity->ReadComponent<RigidBody2D>().bullet;
    });
  }
  
  void NativeGet2DColliderOffset(uint64_t id , glm::vec2* out_offset) {
    NativeDoThing<void>(id , [&out_offset](Entity* entity) {
      if (!entity->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving density from entity without a collider : {}" , entity->Name());
      }

      *out_offset = entity->ReadComponent<Collider2D>().offset;
    });
  }

  void NativeGet2DColliderSize(uint64_t id , glm::vec2* out_size) {
    NativeDoThing<void>(id , [&out_size](Entity* entity) {
      if (!entity->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving density from entity without a collider : {}" , entity->Name());
      }

      *out_size = entity->ReadComponent<Collider2D>().size;
    });
  }

  float NativeGet2DColliderDensity(uint64_t id) {
    return NativeDoThing<float>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving density from entity without a collider : {}" , entity->Name());
        return 0.f;
      }

      return entity->ReadComponent<Collider2D>().density;
    });
  }

  float NativeGet2DColliderFriction(uint64_t id) {
    return NativeDoThing<float>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving friction from entity without a collider : {}" , entity->Name());
        return 0.f;
      }

      return entity->ReadComponent<Collider2D>().friction;
    });
  }

  void NativeSetLinearDrag(uint64_t id , float* linear_drag) {
    NativeDoThing<void>(id , [&linear_drag](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving linear_drag from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().linear_drag = *linear_drag;
    });
  }

  void NativeSetAngularDrag(uint64_t id , float* angular_drag) {
    NativeDoThing<void>(id , [&angular_drag](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving angular_drag from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().angular_drag = *angular_drag;
    });
  }

  void NativeSetGravityScale(uint64_t id , float* gravity_scale) {
    NativeDoThing<void>(id , [&gravity_scale](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving gravity_scale from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().gravity_scale = *gravity_scale;
    });
  }

  void NativeSetFixedRotation(uint64_t id , bool* fixed_rot) {
    NativeDoThing<void>(id , [&fixed_rot](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving fixed_rotation from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().fixed_rotation = *fixed_rot;
    });
  }

  void NativeSetBullet(uint64_t id , bool* bullet) {
    NativeDoThing<void>(id , [&bullet](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving bullet from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().bullet = *bullet;
    });
  }
  
  void NativeSet2DColliderOffset(uint64_t id , glm::vec2* offset) {
    NativeDoThing<void>(id , [&offset](Entity* ent) {
      if (!ent->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving offset from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<Collider2D>().offset = *offset;
    });
  }

  void NativeSet2DColliderSize(uint64_t id , glm::vec2* size) {
    NativeDoThing<void>(id , [&size](Entity* ent) {
      if (!ent->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving size from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<Collider2D>().size = *size;
    });
  }

  void NativeSet2DColliderDensity(uint64_t id , float* density) {
    NativeDoThing<void>(id , [&density](Entity* ent) {
      if (!ent->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving density from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<Collider2D>().density = *density;
    });
  }

  void NativeSet2DColliderFriction(uint64_t id , float* friction) {
    NativeDoThing<void>(id , [&friction](Entity* ent) {
      if (!ent->HasComponent<Collider2D>()) {
        OE_ERROR("Retrieving friction from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<Collider2D>().friction = *friction;
    });
  }
  
  void NativeSetStaticMeshHandle(uint64_t id , uint64_t handle) {
    NativeDoThing<void>(id , [&handle](Entity* ent) {
      if (!ent->HasComponent<StaticMesh>()) {
        OE_ERROR("Setting Static Mesh handle on entity without a static mesh : {}" , ent->Name());
        return;
      }

      ent->GetComponent<StaticMesh>().handle = handle;
    });
  }

  void NativeSetStaticMeshMaterial(uint64_t id , Material* material) {
    NativeDoThing<void>(id , [&material](Entity* ent) {
      if (!ent->HasComponent<StaticMesh>()) {
        OE_ERROR("Setting Static Mesh handle on entity without a static mesh : {}" , ent->Name());
        return;
      }

      ent->GetComponent<StaticMesh>().material = *material;
    });
  }

} // namespace cs_script_bindings
} // namespace other
