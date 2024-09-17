/**
 * \file scripting/cs/native_functions/cs_native_entity.hpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_ENTITY_HPP
#define OTHER_ENGINE_CS_NATIVE_ENTITY_HPP

#include <mono/metadata/object.h>

#include <glm/glm.hpp>

#include "rendering/material.hpp"

#include "scripting/cs/native_functions/cs_native_helpers.hpp"

namespace other {
namespace cs_script_bindings {

  /// internal getters
  MonoString* NativeGetName(uint64_t id);

  uint64_t NativeGetObjectByName(MonoString* name);

  bool NativeHasObjectById(uint64_t id);
  bool NativeHasObjectByName(MonoString* name);

  uint64_t NativeGetParent(uint64_t id);

  uint64_t NativeGetNumChildren(uint64_t id);
  MonoArray* NativeGetChildren(uint64_t id);

  void NativeAddComponent(uint64_t id , MonoReflectionType* rtype);
  bool NativeHasComponent(uint64_t id , MonoReflectionType* rtype);
  void NativeRemoveComponent(uint64_t id , MonoReflectionType* rtype);

  uint64_t NativeGetStaticMeshHandle(uint64_t id);
  void NativeGetStaticMeshMaterial(uint64_t id , Material* material);
  
  /// internal setters

  void NativeSetStaticMeshHandle(uint64_t id , uint64_t handle);
  void NativeSetStaticMeshMaterial(uint64_t id , Material* material);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_SCENE_HPP
