/**
 * \file scripting/cs/native_functions/cs_native_input.cpp
 **/
#include "scripting/cs/native_functions/cs_native_input.hpp"

#include "input/mouse.hpp"

namespace other {
namespace cs_script_bindings {

  void MousePos(glm::vec2* pos) {
    *pos = {
      Mouse::GetX() ,
      Mouse::GetY()
    };
  }

  void MousePreviousPos(glm::vec2* last_pos) {
    *last_pos = {
      Mouse::PreviousX() ,
      Mouse::PreviousY()
    };
  }

  void MouseDeltaPos(glm::vec2* delta_pos) {
    *delta_pos = {
      Mouse::GetDX() ,
      Mouse::GetDY()
    };
  }

} // namespace cs_script_bindings
} // namespace other
