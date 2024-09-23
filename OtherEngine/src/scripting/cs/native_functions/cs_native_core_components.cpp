
/**
 * \file scripting/cs/native_functions/cs_native_core_components.cpp
 **/
#include "scripting/cs/native_functions/cs_native_core_components.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"

namespace other {
namespace cs_script_bindings {

  void NativeRotateObject(uint64_t id , float angle , glm::vec3* axis) {
    NativeDoThing<void>(id , [&angle , &axis](Entity* ent) {
      auto& t = ent->GetComponent<Transform>();
      t.Rotate(angle , *axis);
    });
  }

} // namespace cs_script_bindings
} // namespace other
