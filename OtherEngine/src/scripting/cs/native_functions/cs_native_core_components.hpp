
/**
 * \file scripting/cs/native_functions/cs_native_core_components.cpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_CORE_COMPONENTS_HPP
#define OTHER_ENGINE_CS_NATIVE_CORE_COMPONENTS_HPP

#include <cstdint>

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "ecs/components/transform.hpp"

#include "scripting/cs/native_functions/cs_native_helpers.hpp"

namespace other {
namespace cs_script_bindings {

  void NativeRotateObject(uint64_t id , float angle , glm::vec3* axis);

  NATIVE_GET(glm::vec3 , Scale, scale)
  NATIVE_SET(glm::vec3 , Scale, scale)

  NATIVE_GET(glm::vec3 , Position, position)
  NATIVE_SET(glm::vec3 , Position, position)

  NATIVE_GET(glm::vec3 , Rotation, erotation)
  NATIVE_SET(glm::vec3 , Rotation, erotation)

  NATIVE_GET(glm::quat , Quaternion, qrotation)
  NATIVE_SET(glm::quat , Quaternion, qrotation)

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_CORE_COMPONENTS_HPP
