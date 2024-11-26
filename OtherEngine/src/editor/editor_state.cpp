/**
 * \file editor/editor_state.cpp
 **/
#include "editor/editor_state.hpp"

namespace other {

  EditorState& EditorState::Get() {
    static EditorState instance;
    return instance;
  }

}  // namespace other