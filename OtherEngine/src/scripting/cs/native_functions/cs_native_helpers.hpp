
/**
 * \file scripting/cs/native_functions/cs_native_helpers.cpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_HELPERS_HPP
#define OTHER_ENGINE_CS_NATIVE_HELPERS_HPP

#include <cstdint>
#include <type_traits>
#include <vcruntime_typeinfo.h>

#include "core/ref.hpp"

#include "ecs/entity.hpp"

#include "scripting/script_engine.hpp"

namespace other {
namespace cs_script_bindings {

#define NATIVE_GET(type , name, member) \
  static inline void NativeGet##name(uint64_t id , type* out_##member) { \
    NativeDoThing<void>(id , [&out_##member](Entity* ent) { \
      *out_##member = ent->ReadComponent<Transform>().member; \
    }); \
  }

#define NATIVE_SET(type , name, member) \
  static inline void NativeSet##name(uint64_t id , type* member) { \
    NativeDoThing<void>(id , [&member](Entity* ent) { \
      ent->GetComponent<Transform>().member = *member; \
    }); \
  }

  using FunctionHandle = const void*;
  
  template <typename Ret , typename Fn>
  static Ret NativeDoThing(uint64_t entity_id , Fn thing) {
    Ref<Scene> ctx = ScriptEngine::GetSceneContext(); 
    OE_ASSERT(ctx != nullptr , "Scripts active without active scene context! (program metadata {})" , typeid(Fn).name());
    OE_ASSERT(ctx->HasEntity(entity_id) , "Attempting to access entity from C# bindings that does not exist {} (program metadata {})" , entity_id ,
              typeid(Fn).name());

    Entity* entity = ctx->GetEntity(entity_id);
    return thing(entity);
  }

  template <typename Ret , typename C , Ret C::*value>
  struct NativeReturnThing {
    typedef Ret(*type)(uint64_t);
  };

  template <typename Ret , typename C , Ret C::*value>
  struct NativeRetrieveThing {
    typedef void(*type)(uint64_t , Ret*);
  };

  template <typename Ret , typename C , Ret C::*value>
  static Ret NativeReturnThingFn(uint64_t id) {
    return NativeDoThing<Ret>(id , [](Entity* entity) -> Ret {
      if (!entity->HasComponent<C>()) {
        OE_ERROR("Retrieving {} from entity without a {} : {}" , typeid(C).name() , entity->Name());
        return Ret{};
      }

      if constexpr (std::same_as<UUID, Ret>) {
        return entity->ReadComponent<C>().*value.Get();
      } else {
        return entity->ReadComponent<C>().*value;
      }

    });
  }

  template <typename Ret , typename C , Ret C::*target>
  static void NativeRetrieveThingFn(uint64_t id , Ret* out) {
    NativeDoThing<void>(id , [&out](Entity* entity) {
      *out = entity->ReadComponent<C>().*target;
    });
  }

  template <typename VT , typename C , VT C::*target>
  struct NativeSetThing {
    typedef void(*type)(uint64_t , VT*);
  };

  template <typename VT , typename C , VT C::*target>
  static void NativeSetThingFn(uint64_t id , VT* value) {
    NativeDoThing<void>(id , [&value](Entity* entity) {
      if (value == nullptr) {
        OE_ERROR("Attempting to set {} to null on entity {}" , typeid(C).name() , entity->Name());
        return;
      }

      OE_DEBUG("Setting {} on entity {}" , typeid(C).name() , entity->Name());
      entity->GetComponent<C>().*target = *value;
      OE_DEBUG("Set {} on entity {}" , typeid(C).name() , entity->Name());
    });
  }

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_HELPERS_HPP
