/**
 * \file editor/panels/framebuffer_editor.cpp
 **/
#include "editor/panels/framebuffer_editor.hpp"

#include <imgui/imgui.h>

#include "application/app_state.hpp"

namespace other {

  void FramebufferEditor::OnAttach() {
  }

  bool FramebufferEditor::OnGuiRender(bool& is_open) {
    if (!ImGui::Begin("Renderpass Creator", &is_open)) {
      ImGui::End();
      return false;
    }

    Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();

    ImGui::End();
    return false;
  }

}  // namespace other