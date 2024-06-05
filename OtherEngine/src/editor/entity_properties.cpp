/**
 * \file editor/entity_properties.cpp
 **/
#include "editor/entity_properties.hpp"

#include <imgui/imgui.h>

#include "ecs/entity.hpp"
#include "editor/selection_manager.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  void EntityProperties::OnGuiRender(bool& is_open) {
    is_open = SelectionManager::HasSelection();
    if (!is_open) {
      return;
    }

    if (!ImGui::Begin("Properties" , &is_open)) {
      ImGui::End();
      return;
    }

    {
      ScopedColor bg_color(ImGuiCol_WindowBg , ui::theme::background_dark);

      Entity* selection = SelectionManager::ActiveSelection();
      if (selection == nullptr) {
        SelectionManager::ClearSelection();
        ImGui::End();
        return;
      }

      ImGui::Text("%s" , selection->Name().c_str());
    }
    ImGui::End();
  }

  void EntityProperties::OnEvent(Event* e) {

  }

  void EntityProperties::OnProjectChange(const Ref<Project>& project) {

  }

  void EntityProperties::SetSceneContext(const Ref<Scene>& scene) {

  }

} // namespace other
