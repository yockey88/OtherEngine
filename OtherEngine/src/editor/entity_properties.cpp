/**
 * \file editor/entity_properties.cpp
 **/
#include "editor/entity_properties.hpp"

#include <imgui/imgui.h>

#include "input/keyboard.hpp"
#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/mesh.hpp"
#include "editor/selection_manager.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

namespace other {
  
  template <component_type C , typename Func>
  void DrawComponent(const std::string& name , Func ui_function) {
    // bool should_draw = true;

    Entity* selection = SelectionManager::ActiveSelection();
    if (selection == nullptr) {
      return;
    }

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

      const std::string id_str = fmt::format(fmt::runtime("{}.{}") , selection->Name() , "TRANSFORM");
      ImGuiID ent_comp_id = ImGui::GetID(id_str.c_str());

      ImGui::PushID(ent_comp_id);
      open = ImGui::TreeNodeEx(name.c_str() , tree_flags);

      ImGui::PopID();
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
      auto& component = selection->GetComponent<C>();

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
      C& component = selection->GetComponent<C>();
      /// const bool is_multi_edit ??
      ui_function(component , selection);
      ImGui::TreePop();
    }

    if (remove_comp) {
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
        ImGui::End();
        return;
      }

      ImGui::AlignTextToFramePadding();

      ImVec2 avail_region = ImGui::GetContentRegionAvail();

      ui::ShiftCursor(4.f , 4.f);

      bool hovering_id = false;
      // const bool multi_select = ???

      { /// tag
        const float icon_offset = 6.f;
        ui::ShiftCursor(4.f , icon_offset);

        /// draw entity icon here ??
        /// draw image
        // ImGui::SameLine(0.f , 4.f);
        // ui::ShiftCursorY(-icon_offset);
        
        /// necessary to check consistency when multiselect implemented
        // const bool inconsistent_tag = 

        const std::string& tag = selection->Name();

        std::array<char , 256> buffer{};
        std::copy(tag.begin() , tag.end() , buffer.begin());

        ImGui::PushItemWidth(avail_region.x * 0.5f);
        ScopedStyle frame_border(ImGuiStyleVar_FrameBorderSize , 0.f);
        ScopedColor frame_color(ImGuiCol_FrameBg , IM_COL32(0 , 0 , 0 , 0));
        ScopedFont bold_font(ImGui::GetIO().Fonts->Fonts[0]);

        if (Keyboard::Pressed(Keyboard::Key::OE_F2) && !ImGui::IsAnyItemActive()) { // or (viewport focused)
          ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText("##tag" , buffer.data() , buffer.size())) {
          if (buffer[0] == 0) {
            buffer = { "[ Unnamed Entity ]" }; 
          }

          /// set empty name
        }

        // ui::DrawItemActivityOutline(ui::OutlineFlags_NoOutlineInactive);
      
        hovering_id = ImGui::IsItemHovered();

        ImGui::PopItemWidth();
      }

      ImGui::SameLine();
      ui::ShiftCursorX(-5.f);

      float line_height = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.f;

      const std::string add_string = " ADD     ";
      ImVec2 add_text_size = ImGui::CalcTextSize(add_string.c_str());
      add_text_size.x += GImGui->Style.FramePadding.x * 2.f;

      {
        ScopedColorStack add_comp_button_colors(
          ImGuiCol_Button , IM_COL32(70 , 70 , 70 , 200) ,
          ImGuiCol_ButtonHovered , IM_COL32(70 , 70 , 70 , 255) ,
          ImGuiCol_ButtonActive , IM_COL32(70 , 70 , 70 , 150)
        );

        ImGui::SameLine(avail_region.x - (add_text_size.x + GImGui->Style.FramePadding.x));
        if (ImGui::Button(add_string.c_str() , ImVec2(add_text_size.x + 4.f , line_height + 2.f))) {
          ImGui::OpenPopup("##add_component_panel");
        }

        const float pad = 4.f;
        const float icon_width = ImGui::GetFrameHeight() - pad * 2.f;
        const float icon_height = icon_width;

        ImVec2 icon_pos = ImGui::GetItemRectMax();
        icon_pos.x -= icon_width + pad;
        icon_pos.y -= icon_height + pad;

        ImGui::SetCursorScreenPos(icon_pos);
        ui::ShiftCursor(-5.f , 7.f);

        /// plus icon image
      }

      float add_comp_start_y = ImGui::GetCursorScreenPos().y;

      ImGui::Spacing();
      ImGui::Spacing();
      ImGui::Spacing();

      {
        ScopedFont bold_font(ImGui::GetIO().Fonts->Fonts[0]);
        ScopedStyle item_spacing(ImGuiStyleVar_ItemSpacing , ImVec2(0 , 0));
        ScopedStyle window_padding(ImGuiStyleVar_WindowPadding , ImVec2(5 , 10));
        ScopedStyle cell_padding(ImGuiStyleVar_CellPadding , ImVec2(0.f , 0.f));

        static float add_comp_panel_w = 250.f;
        ImVec2 window_pos = ImGui::GetWindowPos();
        const float max_h = ImGui::GetContentRegionMax().y - 60.f;

        ImGui::SetNextWindowPos({ window_pos.x + add_comp_panel_w / 1.3f , add_comp_start_y });
        ImGui::SetNextWindowSizeConstraints({ 50.f , 50.f } , { add_comp_panel_w , max_h });
        if (ImGui::BeginPopup("##add_component_panel" , ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking)) {

          if (ImGui::BeginTable("##add_component_table" , 2 , ImGuiTableFlags_SizingStretchSame)) {
            ImGui::TableSetupColumn("icon" , ImGuiTableColumnFlags_WidthFixed , add_comp_panel_w * 0.15f);
            ImGui::TableSetupColumn("component names" , ImGuiTableColumnFlags_WidthFixed , add_comp_panel_w * 0.85f);
            
            ImGui::EndTable();
          }
          
          ImGui::EndPopup();
        }
      }

      DrawSelectionComponents(selection);
    }
    ImGui::End();
  }

  void EntityProperties::OnEvent(Event* e) {
  }

  void EntityProperties::OnProjectChange(const Ref<Project>& project) {
  }

  void EntityProperties::SetSceneContext(const Ref<Scene>& scene) {
    OE_ASSERT(scene != nullptr , "Setting null scene context in entity properties panel");
    active_scene = scene;
  }
      
  void EntityProperties::DrawSelectionComponents(Entity* entity) {
    DrawComponent<Transform>("Transform" , [&](Transform& transform , Entity* selection) {
      ScopedStyle spacing(ImGuiStyleVar_ItemSpacing , ImVec2(8.f , 8.f));
      ScopedStyle padding(ImGuiStyleVar_FramePadding , ImVec2(4.f , 4.f));

      ImGui::BeginTable("Transform Component" , 2 , ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoClip);
      ImGui::TableSetupColumn("label_col" , 0 , 100.f);
      ImGui::TableSetupColumn("value_col" , ImGuiTableColumnFlags_IndentEnable | ImGuiTableColumnFlags_NoClip , ImGui::GetContentRegionAvail().x - 100.f);

      bool translation_manually_edited = false;
      bool rotation_manually_edited = false;
      bool scale_manually_edited = false;

      /// replace with this input parameter
      bool multi_edit = false;
      if (multi_edit) {
      } else {
        auto& component = selection->GetComponent<Transform>();

        ImGui::TableNextRow();
        ui::widgets::DrawVec3Control("Translation" , component.position , translation_manually_edited);

        ImGui::TableNextRow();
        glm::vec3 erotation = glm::degrees(component.erotation);
        if (ui::widgets::DrawVec3Control("Rotation" , erotation , rotation_manually_edited)) {
          component.erotation = erotation;
          /// recalculate quaternion
        }

        ImGui::TableNextRow();
        ui::widgets::DrawVec3Control("Scale" , component.scale , scale_manually_edited , 1.f);
      }

      ImGui::EndTable();

      ui::ShiftCursorY(-8.f);
      ui::Underline();

      ui::ShiftCursorY(18.f);
    });
  }

} // namespace other
