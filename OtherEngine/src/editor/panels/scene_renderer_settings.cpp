/**
 * \file editor/panels/scene_renderer_settings.cpp
 **/
#include "editor/panels/scene_renderer_settings.hpp"

#include <imgui/imgui.h>

#include "application/app_state.hpp"

namespace other {

  void SceneRendererSettings::OnAttach() {
  }
  void SceneRendererSettings::OnDetach() {
  }

  bool SceneRendererSettings::OnGuiRender(bool& is_open) {
    if (!ImGui::Begin("Scene Renderer Settings", &is_open)) {
      ImGui::End();
      return false;
    }

    Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
    const auto& pipelines = renderer->GetPipelines();
    const auto& passes = renderer->GetRenderPasses();

    for (auto& [id, pl] : pipelines) {
      ImGui::Text("Pipeline: %s", pl->Name().c_str());
    }

    for (auto& [id, pass] : passes) {
      ImGui::Text("Renderpass: %s", pass->Name().c_str());
    }

    ImGui::End();
    return false;
  }

}  // namespace other