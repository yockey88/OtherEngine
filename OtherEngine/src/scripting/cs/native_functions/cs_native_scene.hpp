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

  uint32_t NativeGetPhysicsBodyType(uint64_t id);
  float NativeGetMass(uint64_t id);
  float NativeGetLinearDrag(uint64_t id);
  float NativeGetAngularDrag(uint64_t id);
  float NativeGetGravityScale(uint64_t id);
  bool NativeGetFixedRotation(uint64_t id);
  bool NativeGetBullet(uint64_t id);

  void NativeGet2DColliderOffset(uint64_t id , glm::vec2* out_offset);
  void NativeGet2DColliderSize(uint64_t id , glm::vec2* out_size);
  float NativeGet2DColliderDensity(uint64_t id);
  float NativeGet2DColliderFriction(uint64_t id);
  
  /// internal setters
  void NativeSetScale(uint64_t id , glm::vec3* out_scale);
  void NativeSetPosition(uint64_t id , glm::vec3* out_position);
  void NativeSetRotation(uint64_t id , glm::vec3* out_rotation);
  
  void NativeSetPhysicsBodyType(uint64_t id , uint32_t type);
  void NativeSetMass(uint64_t id , float* mass);
  void NativeSetLinearDrag(uint64_t id , float* linear_drag);
  void NativeSetAngularDrag(uint64_t id , float* angular_drag);
  void NativeSetGravityScale(uint64_t id , float* gravity_scale);
  void NativeSetFixedRotation(uint64_t id , bool* fixed_rot);
  void NativeSetBullet(uint64_t id , bool* bullet);
  
  void NativeSet2DColliderOffset(uint64_t id , glm::vec2* offset);
  void NativeSet2DColliderSize(uint64_t id , glm::vec2* size);
  void NativeSet2DColliderDensity(uint64_t id , float* density);
  void NativeSet2DColliderFriction(uint64_t id , float* friction);

  void NativeAddComponent(uint64_t id , MonoReflectionType* rtype);
  bool NativeHasComponent(uint64_t id , MonoReflectionType* rtype);
  void NativeRemoveComponent(uint64_t id , MonoReflectionType* rtype);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_SCENE_HPP
