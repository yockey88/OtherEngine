/**
 * \file scripting/cs/native_functions/cs_native_input.hpp
 **/
#ifndef OTHER_ENGINE_CS_NATIVE_INPUT_HPP
#define OTHER_ENGINE_CS_NATIVE_INPUT_HPP

#include <glm/glm.hpp>

namespace other {
namespace cs_script_bindings {

  void MousePos(glm::vec2* pos);

  void MousePreviousPos(glm::vec2* last_pos);

  void MouseDeltaPos(glm::vec2* delta_pos);

} // namespace cs_script_bindings
} // namespace other

#endif // !OTHER_ENGINE_CS_NATIVE_INPUT_HPP
