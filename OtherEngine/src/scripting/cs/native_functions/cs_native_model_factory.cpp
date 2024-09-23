/**
 * \file scripting/cs/native_functions/cs_native_model_factory.cpp
 **/
#include "scripting/cs/native_functions/cs_native_model_factory.hpp"

#include "rendering/model_factory.hpp"

namespace other {
namespace cs_script_bindings {

  uint64_t NativeCreateBox(glm::vec3* size) {
    return ModelFactory::CreateBox(*size).Get();
  } 

} // namespace cs_script_bindings
} // namespace other
