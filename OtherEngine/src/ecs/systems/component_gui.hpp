/**
 * \file ecs/systems/component_gui.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_GUI_HPP
#define OTHER_ENGINE_COMPONENT_GUI_HPP

#include <imgui/imgui.h>
#include <type_traits>

#include "input/mouse.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/mesh.hpp"
#include "editor/selection_manager.hpp"

#include "rendering/texture.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  template <component_type C , typename Func>
  void DrawComponent(const std::string& name , Func ui_function) {
    // bool should_draw = true;

    Entity* selection = SelectionManager::ActiveSelection();
    if (selection == nullptr) {
      return;
    }

    if (!selection->HasComponent<C>()) {
      return;
    }

    /// TODO: eliminate rtti here, this is simply the easiest for right now
    ///       without using the name or hashing the name 
    ImGui::PushID((void*)typeid(C).hash_code());
    ImVec2 avail_region = ImGui::GetContentRegionAvail();

    ImGuiTreeNodeFlags tree_flags = ImGuiTreeNodeFlags_Framed |
      ImGuiTreeNodeFlags_SpanAvailWidth |
      ImGuiTreeNodeFlags_AllowItemOverlap |
      ImGuiTreeNodeFlags_FramePadding;

    const float frame_paddingx = 6.f;
    const float frame_paddingy = 6.f;

    bool open = false;
    {
      ScopedStyle header_rounding(ImGuiStyleVar_FrameRounding , 0.f);
      ScopedStyle header_padding_and_height(ImGuiStyleVar_FramePadding , ImVec2{ frame_paddingx , frame_paddingy });
      open = ImGui::TreeNodeEx(name.c_str() , tree_flags);
    }
    bool right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);

    float line_height = ImGui::GetItemRectMax().y - ImGui::GetItemRectMin().y;
    float item_padding = 4.f;

    bool reset_values = false;
    bool remove_comp = false;

    ImGui::SameLine(avail_region.x - line_height - 5.f);
    ui::ShiftCursorY(line_height / 4.f);
    
    if (ImGui::InvisibleButton("##options" , ImVec2{ line_height , line_height }) || right_clicked) {
      ImGui::OpenPopup("##component_settings");
    }

    if (ui::OpenPopup("##component_settings")) {
      /// this should really be submesh?
      if constexpr (!std::is_same_v<C , Mesh>) {
        ui::ShiftCursorX(item_padding);
        if (ImGui::MenuItem("Copy")) {
          /// copy component
        }

        ui::ShiftCursorX(item_padding);
        if (ImGui::MenuItem("Paste")) {
          /// paste copied component
        }
      }

      ui::ShiftCursorX(item_padding);
      if (ImGui::MenuItem("Reset")) {
        reset_values = true;
      }

      ui::ShiftCursorX(item_padding);                
      if constexpr (!std::is_same_v<C , Transform> && !std::is_same_v<C , Relationship>) {
        if (ImGui::MenuItem("Remove Component")) {
          remove_comp = true;
        }
      }

      ui::EndPopup();
    }

    if (open) {
      ui_function(selection);
      ImGui::TreePop();
    }

    if (remove_comp && selection->HasComponent<C>()) {
      selection->RemoveComponent<C>();
    }

    if (reset_values) {      /// also should be submesh
      if constexpr (std::is_same_v<C , Mesh>) {

      } else {
        selection->RemoveComponent<C>();
        selection->AddComponent<C>();
        /// if actually mesh 
      }
    }

    if (!open) {
      ui::ShiftCursorY(-(ImGui::GetStyle().ItemSpacing.y + 1.f));
    }

    ImGui::PopID();
  }

  void DrawTag(Entity* ent);
  void DrawTransform(Entity* ent);
  void DrawRelationship(Entity* ent);
  void DrawScript(Entity* ent); 
  void DrawMesh(Entity* ent);
  void DrawStaticMesh(Entity* ent);
  void DrawCamera(Entity* ent);
  void DrawRigidBody2D(Entity* ent);
  void DrawCollider2D(Entity* ent);
  
  template <component_type C , component_type... ICs>
  void DrawAddComponentButton(const std::string& name , Ref<Texture2D> icon = nullptr) {
    auto selection = SelectionManager::ActiveSelection();
    if (selection == nullptr) {
      ScopedColor red_color(ImGuiCol_Text , ui::theme::red);
      ImGui::Text("Active Selection Corrupted");
      return;
    }

    if (selection->HasComponent<C>()) {
      return;
    }
    
    if (icon == nullptr) {
      // get default asset icon
    }

    const float row_height = 25.f;
    auto* window = ImGui::GetCurrentWindow();

    window->DC.CurrLineSize.y = row_height;

    ImGui::TableNextRow(0 , row_height);
    ImGui::TableSetColumnIndex(0);

    window->DC.CurrLineTextBaseOffset = 3.f;

    const ImVec2 row_area_min = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable() , 0).Min;
    const ImVec2 row_area_max = {
      ImGui::TableGetCellBgRect(ImGui::GetCurrentTable() , ImGui::TableGetColumnCount() - 1).Max.x - 20 ,
      row_area_min.y + row_height
    };

    ImGui::PushClipRect(row_area_min , row_area_max , false);

    bool hovered, held;
    ImGui::ButtonBehavior(ImRect(row_area_min , row_area_max) , ImGui::GetID(name.c_str()) , 
                          &hovered , &held , ImGuiButtonFlags_AllowOverlap);

    ImGui::SetItemAllowOverlap();
    ImGui::PopClipRect();

    auto fill_row_w_color = [](const ImColor& color) {
      for (uint32_t c = 0; c < ImGui::TableGetColumnCount(); ++c) {
        ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg , color , c);      
      }
    };

    if (hovered) {
      fill_row_w_color(ui::theme::background);
    }

    ui::ShiftCursor(1.5f , 1.5f);
    // ui::Image(icon , { row_height - 3.f , row_height - 2.f });
    ui::ShiftCursor(-1.5f , -1.5f);

    ImGui::TableSetColumnIndex(1);
    ImGui::SetNextItemWidth(-1);
    ImGui::TextUnformatted(name.c_str());
    
    if (sizeof...(ICs) > 0 && selection->HasComponent<ICs...>()) {
      return;
    }

    if (hovered && Mouse::Pressed(Mouse::Button::LEFT)) {
      selection->AddComponent<C>();
      ImGui::CloseCurrentPopup();
    }
  }

  template <typename Fn>
  concept entity_modifier = std::is_invocable_v<Fn , Entity*>;

  template <component_type C , entity_modifier Fn , component_type... ICs>
  void DrawAddComponentButton(const std::string& name , Fn on_added , Ref<Texture2D> icon = nullptr) {
    DrawAddComponentButton<C , ICs...>(name);

    /// if we added the component call the function
    if (SelectionManager::HasSelection() && SelectionManager::ActiveSelection()->HasComponent<C>()) {
      on_added(SelectionManager::ActiveSelection());
    }
  }

} // namespace other

#endif // !OTHER_ENGINE_COMPONENT_GUI_HPP
