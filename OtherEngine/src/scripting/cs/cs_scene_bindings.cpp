/**
 * \file scripting/cs/cs_scene_bindings.cpp
 **/
#include "scripting/cs/cs_scene_bindings.hpp"

#include <core/dotother_defines.hpp>

#include "core/logger.hpp"
#include "ecs/entity.hpp"
#include "scene/scene.hpp"

#include "scripting/script_engine.hpp"
#include "scripting/cs/cs_register_internal_call.hpp"

namespace other {
namespace {

  void* GetNativeHandle(uint64_t id) {
    Ref<Scene> scene = ScriptEngine::GetSceneContext();
    if (scene == nullptr) {
      OE_ERROR("Attempting to retrieve native entity handle from invalid scene context!");
      return nullptr;
    }

    Entity* handle = scene->GetEntity(id);
    if (handle == nullptr) {
      OE_ERROR("Failed to retrieve native entity handle from scene context");
    }
    //  else {
    //   OE_DEBUG("Retrieved native entity handle {} [{}] from scene context" , handle->Name() , handle->GetUUID());
    // }

    return handle;
  }

  dotother::nbool32 IsHandleValid(Entity* ent) {
    if (ent == nullptr) {
      return false;
    }

    Ref<Scene> scene = ScriptEngine::GetSceneContext();
    OE_ASSERT(scene != nullptr , "Attempting to validate native entity handle without a valid scene context!");

    bool is_handle_valid = scene->IsHandleValid(ent);
    OE_DEBUG("Validating native entity handle [{}] => [{}] from scene context" , ent->GetUUID() , is_handle_valid);

    return dotother::nbool32(is_handle_valid);
  }

} // anonymous namespace 
namespace cs_script_bindings {

  void RegisterSceneFunctions(ref<Assembly> assembly) {
    RegisterInternalCallAs(assembly , "Scene" , "GetNativeHandle" , (void*)&GetNativeHandle);
    RegisterInternalCallAs(assembly , "Scene" , "IsHandleValid" , (void*)&IsHandleValid);
  }

} // namespace cs_script_bindings 
} // namespace other