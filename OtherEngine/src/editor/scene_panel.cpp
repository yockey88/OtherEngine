/**
 * \file editor/scene_panel.cpp
 **/
#include "editor/scene_panel.hpp"

#include <imgui/imgui.h>

#include "ecs/entity.hpp"

namespace other {

  static std::vector<std::string> things_in_scene{};

  void ScenePanel::OnGuiRender(bool& is_open) {
    if (ImGui::Begin("Hierarchy" , &is_open)) {
      if (active_scene == nullptr) {
        bool drap_drop_open = ImGui::BeginDragDropTarget();

        if (drap_drop_open) {
          const ImGuiPayload* data = ImGui::AcceptDragDropPayload("scene_set");
          active_scene = *static_cast<Ref<Scene>*>(data->Data);
          ImGui::EndDragDropTarget();
        }

        ImGui::End();
        return;
      }

      auto& ents = active_scene->SceneEntities();
      bool drap_drop_open = ImGui::BeginDragDropTarget();


      if (drap_drop_open) {
        const ImGuiPayload* data = ImGui::AcceptDragDropPayload("scene_select");
        things_in_scene.emplace_back(static_cast<const char*>(data->Data) , data->DataSize);

        for (auto& t : things_in_scene) {
          ImGui::Text("%s" , t.c_str());
        }

        ImGui::EndDragDropTarget();
      }
    }
    ImGui::End();
  }

  void ScenePanel::OnEvent(Event* e) {
  }

  void ScenePanel::OnProjectChange(const Ref<Project>& project) {
  }

  void ScenePanel::SetSceneContext(const Ref<Scene>& scene) {
  }

} // namespace other 
