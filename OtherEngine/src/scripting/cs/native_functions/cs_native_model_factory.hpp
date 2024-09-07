/**
 * \file scripting/cs/native_functions/cs_native_model_factory.hpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_MODEL_FACTORY_HPP
#define OTHER_ENGINE_CS_NATIVE_MODEL_FACTORY_HPP

#include <glm/glm.hpp>

namespace other {
namespace cs_script_bindings {

  uint64_t NativeCreateBox(glm::vec3* size); 

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_MODEL_FACTORY_HPP
