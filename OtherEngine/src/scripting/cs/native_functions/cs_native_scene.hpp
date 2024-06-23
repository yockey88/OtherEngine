/**
 * \file scripting/cs/native_functions/cs_native_scene.hpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_SCENE_HPP
#define OTHER_ENGINE_CS_NATIVE_SCENE_HPP

#include <mono/metadata/object.h>

#include <glm/glm.hpp>

namespace other {
namespace cs_script_bindings {

  /// internal getters
  MonoString* NativeGetName(uint64_t id);

  void NativeGetScale(uint64_t id , glm::vec3* out_scale);
  void NativeGetPosition(uint64_t id , glm::vec3* out_position);
  void NativeGetRotation(uint64_t id , glm::vec3* out_rotation);

  uint64_t NativeGetObjectByName(MonoString* name);

  bool NativeHasObjectById(uint64_t id);
  bool NativeHasObjectByName(MonoString* name);

  uint64_t NativeGetParent(uint64_t id);

  uint64_t NativeGetNumChildren(uint64_t id);
  MonoArray* NativeGetChildren(uint64_t id);
  
  /// internal setters
  void NativeSetScale(uint64_t id , glm::vec3* out_scale);
  void NativeSetPosition(uint64_t id , glm::vec3* out_position);
  void NativeSetRotation(uint64_t id , glm::vec3* out_rotation);

  void NativeAddComponent(uint64_t id , MonoReflectionType* rtype);
  bool NativeHasComponent(uint64_t id , MonoReflectionType* rtype);
  void NativeRemoveComponent(uint64_t id , MonoReflectionType* rtype);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_SCENE_HPP
