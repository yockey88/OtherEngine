/**
 * \file editor/scene_panel.cpp
 **/
#include "editor/scene_panel.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/relationship.hpp"
#include "editor/editor.hpp"
#include "editor/selection_manager.hpp"

namespace other {

  static std::vector<std::string> things_in_scene{};

  void ScenePanel::OnGuiRender(bool& is_open) {
    ImGui::SetNextWindowSize(ImVec2(200.f , ImGui::GetContentRegionAvail().y));
    if (!ImGui::Begin("Hierarchy" , &is_open , ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar)) {
      ImGui::End();
      return;
    }
    
    if (active_scene == nullptr) {
      ImRect win_rect = { ImGui::GetWindowContentRegionMin() , ImGui::GetWindowContentRegionMax() }; 
      ImGui::Dummy(win_rect.GetSize());

      if (ImGui::BeginDragDropTarget()) { 
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("project_content_folder" , ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
        if (payload == nullptr) {
          ImGui::EndDragDropTarget();
          ImGui::End();
          return;
        }

        Path path = std::string { 
          static_cast<const char*>(payload->Data) , 
        };

        if (!Filesystem::FileExists(path)) {
          OE_WARN("Can not find file : {}" , path);
        } else if (path.extension() != ".yscn") {
          OE_WARN("Scene hierarchy currently only accepts '.yscn' files, loaded file has {} extension" , 
          path.extension());
        } else {
          OE_INFO("LOADING SCENE : {}" , path);
          GetEditor().LoadScene(path);
        }

        ImGui::EndDragDropTarget();
      }
    } else {
      ImGui::BeginChild("##scene_hierarchy");
      {
        ScopedStyle spacing(ImGuiStyleVar_ItemSpacing , ImVec2(0.f , 0.f));
        ScopedColorStack item_bg(ImGuiCol_Header , IM_COL32_DISABLE ,
                                 ImGuiCol_HeaderActive , IM_COL32_DISABLE);

        for (auto& [id , entity] : active_scene->SceneEntities()) {
          RenderEntity(id , entity);
        }
      }
      ImGui::EndChild();
    }

    ImGui::End();
  }

  void ScenePanel::OnEvent(Event* e) {
  }

  void ScenePanel::OnProjectChange(const Ref<Project>& project) {
  }

  void ScenePanel::SetSceneContext(const Ref<Scene>& scene) {
    OE_ASSERT(scene != nullptr , "Attempting to set null scene context in ScenePanel");
    active_scene = scene;
  }
      
  void ScenePanel::RenderEntity(const UUID& id , Entity* entity) {
    ImGuiID ent_id = ImGui::GetID(entity->Name().c_str());

    const float edge_offset = 4.f;
    const float row_height = 21.f;

    auto* window = ImGui::GetCurrentWindow();
    window->DC.CurrLineSize.y = row_height;
    window->DC.CurrLineTextBaseOffset = 3.f;

    const bool selected = SelectionManager::ActiveSelection() == entity;

    ImGuiTreeNodeFlags flags = (selected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
    flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

    if (!entity->HasComponent<Relationship>()) {
      flags |= ImGuiTreeNodeFlags_Leaf;
    } else if (entity->ReadComponent<Relationship>().children.empty()) {
      flags |= ImGuiTreeNodeFlags_Leaf;    
    }

    const bool window_focused = ImGui::IsWindowFocused();
    /// const auto& editor_settings = GetEditor().GetSettings();

    auto check_if_child_selected = [&](const Entity* ent , auto any_child_selected) -> bool {
      if (!SelectionManager::HasSelection()) {
        return false; 
      }

      auto ent_tag = ent->GetComponent<Tag>();
      auto selection_tag = ent->GetComponent<Tag>();
      if (ent_tag.id == selection_tag.id) {
        return true;
      }

      if (!ent->HasComponent<Relationship>()) {
        return false; 
      }

      const auto& relationships = ent->ReadComponent<Relationship>();
      for (const auto& child_id : relationships.children) {
        auto* child = active_scene->GetEntity(child_id);
        if (child == nullptr) {
          continue;
        }

        if (any_child_selected(child , any_child_selected)) {
          return true;
        }
      }


      return false;
    };

    auto fill_w_color = [](const ImColor& color) {
    };

    if (selected) {
      ImGui::PushStyleColor(ImGuiCol_Text , ui::theme::background_dark);
    } 
    
    bool prev_state = ImGui::TreeNodeBehaviorIsOpen(ent_id);

    const bool child_selected = check_if_child_selected(entity , check_if_child_selected);
    const bool active_selection = entity == SelectionManager::ActiveSelection();
    
    
    // if (item_clicked && !child_selected) {
    //   /// select entity for properties panel
    //   if (SelectionManager::ActiveSelection() == entity) {
    //     SelectionManager::ClearSelection();
    //   } else {
    //     SelectionManager::Select(entity);
    //   }
    // }
    // 
    // if (active_selection || item_clicked) {
    //   if (window_focused) {
    //     fill_w_color(ui::theme::selection);
    //   } else {
    //     const ImColor col = ui::theme::ColorWithMultiplier(ui::theme::selection , 0.8f);
    //     fill_w_color(ui::theme::ColorWithMultipliedSaturation(col , 0.7f));
    //   }

    //   // ImGui::PushStyleColor(ImGuiCol_Text , ui::theme::background_dark);
    // } else if (child_selected) {
    //   fill_w_color(ui::theme::selection_muted);
    // }

    bool open = ImGui::TreeNodeEx(entity->Name().c_str() , flags);
    
    // if (active_selection || item_clicked) {
    //   // ImGui::PopStyleColor();
    // }

    ui::ShiftCursorY(3.0f);

    /// context menu

    if (open) {
      /// render children

      ImGui::TreePop();
    } else if (prev_state) {
      /// entity selected last frame
    }
  }

} // namespace other 
