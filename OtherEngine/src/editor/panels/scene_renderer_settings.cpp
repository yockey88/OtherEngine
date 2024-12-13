/**
 * \file editor/panels/scene_renderer_settings.cpp
 **/
#include "editor/panels/scene_renderer_settings.hpp"

#include <imgui/imgui.h>

#include "application/app_state.hpp"

#include "rendering/ui/ui_helpers.hpp"

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

    // Ref<SceneRenderer> renderer = AppState::Scenes()->GetRenderer();
    // auto& pipelines = renderer->GetPipelines();
    // auto& passes = renderer->GetRenderPasses();

    // for (auto& [id, pass] : passes) {
    //   ImGui::Text("Renderpass: %s", pass->Name().c_str());
    //   if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    //     ImGui::SetDragDropPayload("RENDERPASS", &id, sizeof(UUID));
    //     ImGui::Text("[%s] Drag to attach to pipeline", pass->Name().c_str());
    //     ImGui::EndDragDropSource();
    //   }
    // }

    // for (auto& [id, pl] : pipelines) {
    //   ImGui::PushID(id.Get());
    //   if (ui::BeginTreeNode(pl->Name().c_str(), false)) {
    //     for (auto& p : pl->GetRenderPasses()) {
    //       ImGui::Text("Renderpass: %s", p->Name().c_str());
    //     }

    //     if (ImGui::BeginDragDropTarget()) {
    //       if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RENDERPASS")) {
    //         UUID pass_id = *(UUID*)payload->Data;
    //         auto itr = passes.find(pass_id);
    //         if (itr != passes.end()) {
    //           pl->SubmitRenderPass(itr->second);
    //         }
    //       }
    //       ImGui::EndDragDropTarget();
    //     }

    //     ui::EndTreeNode();
    //   }
    //   ImGui::PopID();
    // }

    ImGui::End();
    return false;
  }

}  // namespace other