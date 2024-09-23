
/**
 * \file scripting/cs/native_functions/cs_native_physics_components.cpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_PHYSICS_COMPONENTS_HPP
#define OTHER_ENGINE_CS_NATIVE_PHYSICS_COMPONENTS_HPP

#include <cstdint>

#include <glm/glm.hpp>

namespace other {
namespace cs_script_bindings {

  uint32_t NativeGetPhysicsBodyType(uint64_t id);
  void NativeSetPhysicsBodyType(uint64_t id , uint32_t type);

  float NativeGetMass(uint64_t id);
  void NativeSetMass(uint64_t id , float* mass);

  float NativeGetLinearDrag(uint64_t id);
  void NativeSetLinearDrag(uint64_t id , float* linear_drag);

  float NativeGetAngularDrag(uint64_t id);
  void NativeSetAngularDrag(uint64_t id , float* angular_drag);

  float NativeGetGravityScale(uint64_t id);
  void NativeSetGravityScale(uint64_t id , float* gravity_scale);

  bool NativeGetFixedRotation(uint64_t id);
  void NativeSetFixedRotation(uint64_t id , bool* fixed_rot);

  bool NativeGetBullet(uint64_t id);
  void NativeSetBullet(uint64_t id , bool* bullet);

  void NativeGet2DColliderOffset(uint64_t id , glm::vec2* out_offset);
  void NativeSet2DColliderOffset(uint64_t id , glm::vec2* offset);

  void NativeGet2DColliderSize(uint64_t id , glm::vec2* out_size);
  void NativeSet2DColliderSize(uint64_t id , glm::vec2* size);

  float NativeGet2DColliderDensity(uint64_t id);
  void NativeSet2DColliderDensity(uint64_t id , float* density);

  float NativeGet2DColliderFriction(uint64_t id);
  void NativeSet2DColliderFriction(uint64_t id , float* friction);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_PHYSICS_COMPONENTS_HPP
