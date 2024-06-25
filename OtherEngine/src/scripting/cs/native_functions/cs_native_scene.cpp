/**
 * \file scripting/cs/native_functions/cs_native_scene.cpp
 **/
#include "scripting/cs/native_functions/cs_native_scene.hpp"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "ecs/entity.hpp"

#include "ecs/components/tag.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/collider_2d.hpp"

#include "scripting/script_engine.hpp"
#include "scripting/cs/mono_utils.hpp"
#include "scripting/cs/cs_script_bindings.hpp"

namespace other {
namespace cs_script_bindings {

  template <typename Ret , typename Fn>
  Ret NativeDoThing(uint64_t entity_id , Fn thing) {
    Ref<Scene> ctx = ScriptEngine::GetSceneContext(); 
    OE_ASSERT(ctx != nullptr , "Scripts active without active scene context!");
    OE_ASSERT(ctx->HasEntity(entity_id) , "Attempting to access entity from C# bindings that does not exist {}" , entity_id);

    Entity* entity = ctx->GetEntity(entity_id);
    
    return thing(entity);
  }

  MonoString* NativeGetName(uint64_t id) {
    return NativeDoThing<MonoString*>(id , [](Entity* entity) {
      Ref<CsModule> cs_module = ScriptEngine::GetModuleAs<CsModule>(CS_MODULE);

      std::string name = entity->ReadComponent<Tag>().name;

      MonoString* mname = mono_string_new(cs_module->AppDomain() , name.c_str());
      return mname;
    });
  }
  
  uint64_t NativeGetObjectByName(MonoString* name) {
    Ref<Scene> ctx = ScriptEngine::GetSceneContext();
    OE_ASSERT(ctx != nullptr , "Scripts active without active scene context!");

    char* cname = mono_string_to_utf8(name);
    std::string str{ cname };
    
    uint64_t res = 0;
    if (ctx->HasEntity(cname)) {
      Entity* e = ctx->GetEntity(cname);
      res = e->ReadComponent<Tag>().id.Get();
    }
    mono_free(cname);

    return res;
  }
  
  bool NativeHasObjectById(uint64_t id) {
    Ref<Scene> ctx = ScriptEngine::GetSceneContext();
    OE_ASSERT(ctx != nullptr , "Scripts active without active scene context!");

    return ctx->HasEntity(id);
  }
  
  bool NativeHasObjectByName(MonoString* name) {
    Ref<Scene> ctx = ScriptEngine::GetSceneContext();
    OE_ASSERT(ctx != nullptr , "Scripts active without active scene context!");

    char* cname = mono_string_to_utf8(name);
    std::string str{ cname };
    bool res = ctx->HasEntity(cname);
    mono_free(cname);

    return res;
  }
  
  void NativeGetScale(uint64_t id , glm::vec3* out_scale) {
    NativeDoThing<void>(id , [&out_scale](Entity* ent) {
      *out_scale = ent->ReadComponent<Transform>().scale;
    });
  }

  void NativeGetPosition(uint64_t id , glm::vec3* out_position) {
    NativeDoThing<void>(id , [&out_position](Entity* ent) {
      *out_position = ent->ReadComponent<Transform>().position;
    });
  }

  void NativeGetRotation(uint64_t id , glm::vec3* out_rotation) {
    NativeDoThing<void>(id , [&out_rotation](Entity* ent) {
      *out_rotation = ent->ReadComponent<Transform>().erotation;
    });
  }
  
  uint64_t NativeGetParent(uint64_t id) {
    return NativeDoThing<uint64_t>(id , [](Entity* entity) -> uint64_t {
      const auto& relations = entity->ReadComponent<Relationship>();
      if (!relations.parent.has_value()) {
        return 0;
      }

      return relations.parent.value().Get();
    });
  }
  
  uint64_t NativeGetNumChildren(uint64_t id) {
    return NativeDoThing<uint64_t>(id , [](Entity* entity) -> uint64_t {
      const auto& relations = entity->ReadComponent<Relationship>();
      return relations.children.size();
    });
  }
  
  MonoArray* NativeGetChildren(uint64_t id) {
    return NativeDoThing<MonoArray*>(id , [](Entity* entity) {
      Ref<CsModule> cs_module = ScriptEngine::GetModuleAs<CsModule>(CS_MODULE);
      auto& relations = entity->ReadComponent<Relationship>();

      MonoArray* id_arr = mono_array_new(cs_module->AppDomain() , MonoClassFromValueType(ValueType::UINT64) , 
                                         relations.children.size());

      uint32_t i = 0;
      for (auto& c : relations.children) {
        mono_array_set(id_arr , uint64_t , i , c.Get());
        ++i;
      }
      return id_arr;
    });
  }
  
  uint32_t NativeGetPhysicsBodyType(uint64_t id) {
    return NativeDoThing<uint32_t>(id , [](Entity* ent) -> uint32_t {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving rigid body type from entity without a rigid body : {}" , ent->Name());
        return 0;
      }

      return ent->ReadComponent<RigidBody2D>().type;
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
  
  void NativeSetScale(uint64_t id , glm::vec3* scale) {
    NativeDoThing<void>(id , [&scale](Entity* ent) {
      ent->GetComponent<Transform>().scale = *scale;
    });
  }

  void NativeSetPosition(uint64_t id , glm::vec3* position) {
    NativeDoThing<void>(id , [&position](Entity* ent) {
      ent->GetComponent<Transform>().position = *position;
    });
  }
  
  void NativeSetRotation(uint64_t id , glm::vec3* rotation) {
    NativeDoThing<void>(id , [&rotation](Entity* ent) {
      auto& t = ent->GetComponent<Transform>();
      t.erotation = *rotation;
      /// t.CalcQuat(); 
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
  
  void NativeSetMass(uint64_t id , float* mass) {
    NativeDoThing<void>(id , [&mass](Entity* ent) {
      if (!ent->HasComponent<RigidBody2D>()) {
        OE_ERROR("Retrieving mass from entity without a rigid body : {}" , ent->Name());
        return;
      }
      
      ent->GetComponent<RigidBody2D>().mass = *mass;
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
  
  void NativeAddComponent(uint64_t id , MonoReflectionType* rtype) {
    NativeDoThing<void>(id , [&rtype](Entity* entity) {
      MonoType* mtype = mono_reflection_type_get_type(rtype);

      auto adder = CsScriptBindings::GetBuilder(mtype);
      if (adder == nullptr) {
        const char* type_name = mono_type_get_name(mtype);
        OE_ERROR("Failed to add component {} to entity {}!" , type_name , entity->Name());
        return;
      }

      adder(entity);
    }); 
  }
  
  bool NativeHasComponent(uint64_t id , MonoReflectionType* rtype) {
    return NativeDoThing<bool>(id , [&rtype](Entity* entity) -> bool {
      MonoType* mtype = mono_reflection_type_get_type(rtype);

      auto checker = CsScriptBindings::GetChecker(mtype);
      if (checker == nullptr) {
        const char* type_name = mono_type_get_name(mtype);
        OE_ERROR("Failed to retrieve component checker for component {}!" , type_name);
        return false;
      }

      return checker(entity);
    }); 
  }

  void NativeRemoveComponent(uint64_t id , MonoReflectionType* rtype) {
    NativeDoThing<void>(id , [&rtype](Entity* entity) {
      MonoType* mtype = mono_reflection_type_get_type(rtype);

      auto remover = CsScriptBindings::GetDestroyer(mtype);
      if (remover == nullptr) {
        const char* type_name = mono_type_get_name(mtype);
        OE_ERROR("Failed to retrieve component remover for component {}!" , type_name);
        return;
      }

      remover(entity);
    }); 
  }

} // namespace cs_script_bindings
} // namespace other
