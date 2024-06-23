/**
 * \file scripting/cs/native_functions/cs_native_scene.cpp
 **/
#include "scripting/cs/native_functions/cs_native_scene.hpp"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include "ecs/entity.hpp"

#include "ecs/components/tag.hpp"
#include "ecs/components/script.hpp"

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
