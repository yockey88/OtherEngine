/**
 * \file ecs/systems/component_gui.hpp
 **/
#ifndef OTHER_ENGINE_COMPONENT_GUI_HPP
#define OTHER_ENGINE_COMPONENT_GUI_HPP

#include <imgui/imgui.h>

#include "ecs/entity.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/mesh.hpp"
#include "editor/selection_manager.hpp"
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

      ui::ShiftCursorX(item_padding);                /// should also be submesh
      if constexpr (!std::is_same_v<C , Transform> && !std::is_same_v<C , Relationship> && !std::is_same_v<C , Mesh>) {
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

  void DrawTransform(Entity* entity);

  void DrawRelationship(Entity* ent);

  void DrawScript(Entity* ent); 

  void DrawMesh(Entity* ent);

} // namespace other

#endif // !OTHER_ENGINE_COMPONENT_GUI_HPP
