/**
 * \file scripting/cs/native_functions/cs_native_scene.cpp
 **/
#include "scripting/cs/native_functions/cs_native_entity.hpp"

#include <mono/metadata/object.h>
#include <mono/metadata/reflection.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/ext/quaternion_common.hpp>

#include "ecs/entity.hpp"

#include "ecs/components/tag.hpp"

#include "scripting/script_engine.hpp"
#include "scripting/cs/mono_utils.hpp"
#include "scripting/cs/cs_script_bindings.hpp"

#include "scripting/cs/native_functions/cs_native_helpers.hpp"

namespace other {
namespace cs_script_bindings {

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
  
  uint64_t NativeGetStaticMeshHandle(uint64_t id) {
    return NativeDoThing<uint64_t>(id , [](Entity* entity) -> float {
      if (!entity->HasComponent<StaticMesh>()) {
        OE_ERROR("Retrieving static mesh from entity without a collider : {}" , entity->Name());
        return 0;
      }

      return entity->ReadComponent<StaticMesh>().handle.Get();
    });
  }
  
  void NativeGetStaticMeshMaterial(uint64_t id , Material* material) {
    NativeDoThing<void>(id , [&](Entity* entity) {
      if (!entity->HasComponent<StaticMesh>()) {
        return;
      }

      *material = entity->GetComponent<StaticMesh>().material;
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
