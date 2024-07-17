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
#include "ecs/components/camera.hpp"

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
        const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("project-scenes-folder" , ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
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
      if (ImGui::Button("Unload Active scene")) {
        GetEditor().UnloadScene();
        ImGui::End();
        return;
      }

      ImGuiTableFlags table_flags = ImGuiTableFlags_NoPadInnerX |
        ImGuiTableFlags_Reorderable | 
        ImGuiTableFlags_ScrollY;

      ImRect window_rect = {
        ImGui::GetWindowContentRegionMin() , 
        ImGui::GetWindowContentRegionMax()
      };

      const float edge_offset = 4.f;

      ScopedColor table_bg(ImGuiCol_ChildBg , ui::theme::background_dark);

      const int32_t num_columns = 2;
      if (ImGui::BeginTable("##scene_entities" , num_columns , table_flags , ImVec2(ImGui::GetContentRegionAvail()))) {
        ImGui::TableSetupColumn("Num Children");
        ImGui::TableSetupColumn("Objects");

        /// headers 
        {
          const ImU32 active_color = ui::theme::ColorWithMultiplier(ui::theme::group_header, 1.2f);
          ScopedColorStack header_cols(
            ImGuiCol_HeaderHovered , active_color ,
            ImGuiCol_HeaderActive , active_color
          );

          ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount() , 1);

          ImGui::TableNextRow(ImGuiTableRowFlags_Headers , 22.f);
          for (int32_t col = 0; col < ImGui::TableGetColumnCount(); ++col) {
            ImGui::TableSetColumnIndex(col);
            const char* col_name = ImGui::TableGetColumnName(col);
            ScopedID col_id(col);

            ui::ShiftCursor(edge_offset * 3.f , edge_offset * 2.f);
            ImGui::TableHeader(col_name);
            ui::ShiftCursor(-edge_offset * 3.f , -edge_offset * 2.f);
          }
          ImGui::SetCursorPosX(ImGui::GetCurrentTable()->OuterRect.Min.x);
          ui::Underline(true , 0.f , 5.f);
        }

        ImGui::TableNextRow();

        /// hierarchy
        {
          ScopedColorStack entity_select(
            ImGuiCol_Header , IM_COL32_DISABLE ,
            ImGuiCol_HeaderHovered , IM_COL32_DISABLE ,
            ImGuiCol_HeaderActive , IM_COL32_DISABLE
          );

          auto ents = active_scene->RootEntities();
          for (auto itr = ents.begin(); itr != ents.end();) {
            auto& [id , entity] = *itr;
            if (!RenderEntity(id , entity)) {
              /// TODO: fix this bc it causes flicker of all ents when deleting one;
              break;
            }

            ++itr;
          }
        }
      } else {
        ImGui::Text("Failed to draw entity hierarchy");
      }

      if (ImGui::BeginPopupContextWindow(nullptr , ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        RenderCreateEntity();
        ImGui::EndPopup();
      }

      ImGui::EndTable();
    }

    ImGui::End();
  }

  void ScenePanel::OnEvent(Event* e) {
  }

  void ScenePanel::OnProjectChange(const Ref<Project>& project) {
  }

  void ScenePanel::SetSceneContext(const Ref<Scene>& scene) {
    if (scene == nullptr) {
      OE_DEBUG("Unloading scene");
      active_scene = nullptr;
      return;
    }

    OE_DEBUG("ScenePanel::SetSceneContext(entities = {})" , scene->SceneEntities().size());
    active_scene = scene;

    for (auto& [id , ent] : active_scene->SceneEntities()) {
      OE_DEBUG("Entity : {}" , ent->Name());
    }
  }
      
  void ScenePanel::RenderCreateEntity(Entity* parent) {
    const bool child_entity = parent != nullptr;

    if (!ImGui::BeginMenu("Create")) {
      return;
    }

    Entity* entity = nullptr;
    if (ImGui::MenuItem("Empty Object")) {
      entity = active_scene->CreateEntity(); 
    }

    if (ImGui::MenuItem("Camera")) {
      entity = active_scene->CreateEntity(fmtstr("camera {}" , active_scene->NumCameras())); 
      entity->AddComponent<Camera>();
    }

    if (entity == nullptr) {
      ImGui::EndMenu();
      return;
    }

    const auto& tag = entity->GetComponent<Tag>();

    if (child_entity) {
      active_scene->ParentEntity(tag.id , parent->GetComponent<Tag>().id); 
    }

    ImGui::EndMenu();
  }
      
  bool ScenePanel::RenderEntity(const UUID& id , Entity* entity) {
    OE_ASSERT(entity != nullptr , "DRAWING NULL ENTITY IN HIERARCHY");

    auto& tag = entity->GetComponent<Tag>();
    auto& relationships = entity->GetComponent<Relationship>();
    const std::string ent_name = tag.name;

    // const float edge_offset = 4.f;
    const float row_height = 21.f;

    auto* window = ImGui::GetCurrentWindow();
    window->DC.CurrLineSize.y = row_height;

    ImGui::TableNextRow(0 , row_height);

    ImGui::TableSetColumnIndex(0);
    
    ImGui::Text("  [%lld]" , relationships.children.size());

    ImGui::TableSetColumnIndex(1);

    window->DC.CurrLineTextBaseOffset = 3.f;

    const ImVec2 row_area_min = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable() , 0).Min;
    const ImVec2 row_area_max = {
      ImGui::TableGetCellBgRect(ImGui::GetCurrentTable() , ImGui::TableGetColumnCount() - 1).Max.x - 20 ,
      row_area_min.y + row_height
    };

    const bool active_selection = SelectionManager::HasSelection() ? 
      tag.id == SelectionManager::ActiveSelection()->ReadComponent<Tag>().id :
      false;

    ImGuiTreeNodeFlags flags = (active_selection ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;

    // if has_child_matching_search
    //    flags |= ImGuiTreeNodeFlags_DefaultOpen;

    if (relationships.children.empty()) {
      flags |= ImGuiTreeNodeFlags_Leaf;
    }

    ImGui::PushClipRect(row_area_min , row_area_max , false);
    bool is_row_hovered , held;
    bool is_row_clicked = ImGui::ButtonBehavior(
      ImRect(row_area_min , row_area_max) , 
      ImGui::GetID(ent_name.c_str()) ,
      &is_row_hovered , &held ,
      ImGuiButtonFlags_AllowOverlap |
        ImGuiButtonFlags_PressedOnClickRelease |
        ImGuiButtonFlags_MouseButtonLeft |
        ImGuiButtonFlags_MouseButtonRight
    );

    bool right_clicked = ImGui::IsMouseReleased(ImGuiMouseButton_Right);

    ImGui::SetItemAllowOverlap();

    ImGui::PopClipRect();

    const bool window_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    /// const auto& editor_settings = GetEditor().GetSettings();

    auto fill_row_w_color = [](const ImColor& color) {
      for (int32_t col = 0; col < ImGui::TableGetColumnCount(); ++col) {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg , color , col);
      }
    };

    auto check_if_child_selected = [&](const Entity* ent , auto any_child_selected) -> bool {
      if (!SelectionManager::HasSelection()) {
        return false; 
      }

      auto ent_id = ent->ReadComponent<Tag>().id;
      auto selection_id = SelectionManager::ActiveSelection()->ReadComponent<Tag>().id;
      if (ent_id == selection_id) {
        return true;
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

    const bool child_selected = check_if_child_selected(entity , check_if_child_selected); 

    if (active_selection) {
      if (window_focused /* ui::NavigatedTo()? */) {
        fill_row_w_color(ui::theme::selection);
      } else {
        const ImColor col = ui::theme::ColorWithMultiplier(ui::theme::selection , 0.9f);
        fill_row_w_color(ui::theme::ColorWithMultipliedSaturation(col , 0.7f));
      } 

      ImGui::PushStyleColor(ImGuiCol_Text , ui::theme::background_dark);
    } else if (is_row_hovered) {
      fill_row_w_color(ui::theme::group_header);
    } else if (child_selected) {
      fill_row_w_color(ui::theme::selection_muted);
    }

    ImGuiContext& g = *GImGui;
    auto& style = ImGui::GetStyle();
    // const ImVec2 label_size = ImGui::CalcTextSize(tag.name.c_str(), nullptr, false);

    const ImVec2 padding = ((flags & ImGuiTreeNodeFlags_FramePadding)) ? 
      style.FramePadding : 
      ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));
    
    // Collapser arrow width + Spacing
    const float text_offset_x = g.FontSize + padding.x * 2;

    // Latch before ItemSize changes it
    const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);

    // Include collapser
    // const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);
    ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
    const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
    const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
    const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);

    bool prev_node_state = ImGui::TreeNodeBehaviorIsOpen(ImGui::GetID(tag.name.c_str()));

    if (is_mouse_x_over_arrow && is_row_clicked) {
      ImGui::SetNextItemOpen(!prev_node_state);
    }

    if (!active_selection && child_selected) {
      ImGui::SetNextItemOpen(true);
    }

    const bool open = ImGui::TreeNodeEx(tag.name.c_str() , flags);
    
    int32_t row_idx = ImGui::TableGetRowIndex();
    if (row_idx >= first_selected_row && row_idx <= last_selected_row && !active_selection && shift_selection_running) {
      SelectionManager::Select(entity);

      /// count selections
    }

    float line_height = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;

    auto avail_region = ImGui::GetContentRegionAvail();

    ImGui::SameLine(avail_region.x - line_height - 5.f);
    ui::ShiftCursorY(line_height / 4.f);
    
    bool remove_entity = false;
    if (ImGui::InvisibleButton("##entity-options" , ImVec2{ line_height , line_height }) || right_clicked) {
      ImGui::OpenPopup("##entity-settings");
    }
    
    if (ui::OpenPopup("##entity-settings")) {
      /// pop the background dark for the text so the menu is visible
      if (active_selection) {
        ImGui::PopStyleColor();
      }

      if (ImGui::MenuItem("Destroy Entity")) {
        remove_entity = true;
      }

      if (active_selection) {
        ImGui::PushStyleColor(ImGuiCol_Text , ui::theme::background_dark);
      }
      ui::EndPopup();
    }

    if (is_row_clicked && !right_clicked) {
      /// bool ctrl_down
      /// bool shift_down
      /// if (shift_down && SelectionManager::SelectionCount() > 0) {
      ///   SelectionManager::DeselectAll();
      ///
      ///   if (row_idx < first_row_selected) {
      ///     last_row_selected = first_row_selected;
      ///     first_row_selected = row_idx;
      ///   } else {
      ///     last_row_selected = row_idx;
      ///   }
      ///
      ///   shift_selection_running
      /// } else if (!ctrl_down || shift_down) {
      ///   SelectionManager::DeselectAll();
      ///   SelectManager::Select(entity);
      ///   first_row_selected = row_idx;
      ///   last_row_selected = -1;
      /// } else 

      if (active_selection) {
        SelectionManager::ClearSelection();
      } else {
        SelectionManager::Select(entity);
      }

      ImGui::FocusWindow(ImGui::GetCurrentWindow());
    }

    /// entity selection stuff for selecting more than one

    if (active_selection) {
      ImGui::PopStyleColor();
    }

    /// entity component drag drop sections

    if (open) {
      /// draw children
      auto& relationship = entity->GetComponent<Relationship>();
      for (auto& child : relationship.children) {
        auto* child_ent = active_scene->GetEntity(child);
        RenderEntity(child , child_ent);
      }

      ImGui::TreePop();
    }    

    if (remove_entity) {
      active_scene->DestroyEntity(entity);
      return false;
    }

    return true;
  }

} // namespace other 
