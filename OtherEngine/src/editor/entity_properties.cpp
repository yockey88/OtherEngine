/**
 * \file editor/entity_properties.cpp
 **/
#include "editor/entity_properties.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <type_traits>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/systems/component_gui.hpp"

#include "editor/selection_manager.hpp"

#include "rendering/texture.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {
  
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

        if (ImGui::InputText("##entity-tag" , buffer.data() , buffer.size())) {
          if (buffer[0] == 0) {
            buffer = { "[ Unnamed Entity ]" }; 
          }

          if (ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            /// copy buffer into string
            UUID old_id = selection->ReadComponent<Tag>().id;

            bool name_valid = true;
            std::string name;

            for (auto& c : buffer) {
              if (c == 0) {
                break;
              }
              name += c;
            }

            if (name.empty()) {
              OE_WARN("Can not give entity an empty name!");
              name_valid = false;
            }
            
            UUID id = FNV(name);
            if (name_valid && active_scene->GetEntity(id) != nullptr) {
              OE_WARN("Can not give entity the same name as another entity");
              name_valid = false;
            }

            if (name_valid) {
              active_scene->RenameEntity(old_id , id , name);
            }
          }
        }

        ui::DrawItemActivityOutline(ui::OutlineFlags_NoOutlineInactive);
      
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

            DrawAddComponentButton<Script>("Script");
            DrawAddComponentButton<Mesh>("Mesh");
            DrawAddComponentButton<Camera>("Camera");
            
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
    active_scene = scene;
    if (active_scene == nullptr) {
      SelectionManager::ClearSelection();
    }
  }
      
  void EntityProperties::DrawSelectionComponents(Entity* entity) {
    DrawComponent<Tag>("Tag" , DrawTag);
    DrawComponent<Transform>("Transform" , DrawTransform);
    DrawComponent<Relationship>("Scene Relationships" , DrawRelationship);
    DrawComponent<Script>("Script" , DrawScript);
    DrawComponent<Mesh>("Mesh" , DrawMesh);
    DrawComponent<Camera>("Camera" , DrawCamera);
  }

} // namespace other
