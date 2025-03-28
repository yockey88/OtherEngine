/**
 * \file editor/editor_state.cpp
 **/
#include "editor/editor_state.hpp"

namespace other {

  static ArenaAllocator<EditorState> editor_state_allocator;
  static EditorState* editor_state = nullptr;

  EditorState& EditorState::Get() {
    if (editor_state == nullptr) {
      editor_state = editor_state_allocator.Allocate();
    }
    return *editor_state;
  }

  void EditorState::Shutdown() {
    if (editor_state != nullptr) {
      editor_state_allocator.Free(editor_state);
    }
    editor_state = nullptr;
  }

}  // namespace other