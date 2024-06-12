/**
 * \file editor/entity_properties.cpp
 **/
#include "editor/entity_properties.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>

#include "input/keyboard.hpp"
#include "ecs/entity.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/script.hpp"
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
    active_scene = scene;
    if (active_scene == nullptr) {
      SelectionManager::ClearSelection();
    }
  }
      
  void EntityProperties::DrawSelectionComponents(Entity* entity) {
    

    DrawComponent<Transform>("Transform" , [](Entity* selection) {
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
        if (ui::widgets::DrawVec3Control("Rotation" , component.erotation , rotation_manually_edited)) {
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

    DrawComponent<Relationship>("Scene Relationships" , [](Entity* selection) {
      auto& relations = selection->GetComponent<Relationship>();

      if (!relations.parent.has_value()) {
        ImGui::Text("Root Entity");
      } else {
        ImGui::Text("Parent Entity > [ %lld ]" , relations.parent.value().Get());
      }

      ImGui::Text("Put options related to how parents and children interact");
      ImGui::Text(" > have children inherit parent position? ");
      ImGui::Text(" > other things...");
    });

    DrawComponent<Script>("Script" , [&](Entity* selection) {
      auto& script = selection->GetComponent<Script>();
 
      for (auto& [id , s] : script.scripts) {
        bool is_error = s->IsCorrupt();
        if (is_error) {
          ImGui::Text("%s corrupt, recompile or reload" , s->Name().c_str());

          if (ImGui::Button(("Reload Script##" + s->Name()).c_str())) {
            OE_INFO("Reloading script : {}" , s->Name());
          }
          return;
        }
      
        ui::BeginPropertyGrid();
        ui::ShiftCursor(10.f , 9.f);
        ImGui::Text("%s [ %s ]" , s->Name().c_str() , s->LanguageName().c_str());

        ImGui::NextColumn();
        ui::ShiftCursorY(4.f);
        ImGui::PushItemWidth(-1);

        ImVec2 og_button_txt_align = ImGui::GetStyle().ButtonTextAlign;
        {
          ImGui::GetStyle().ButtonTextAlign = { 0.f , 0.5f };
          float width = ImGui::GetContentRegionAvail().x;
          float item_height = 28.f;

          std::string button_txt = "None";
          bool valid = true;
          /// get full name
          /// button_txt = full_name

          if ((GImGui->CurrentItemFlags & ImGuiItemFlags_MixedValue) != 0) {
            button_txt = "---";
          }

          {
            ScopedColor bg_col(ImGuiCol_WindowBg , ui::theme::property_field);
            ScopedColor button_label_col(ImGuiCol_Text , ui::theme::text);
            ImGui::Button(button_txt.c_str() , { width , item_height });

            const bool hovered = ImGui::IsItemHovered();

            if (hovered) {
              if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                /// open script in text editor
              } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                ImGui::OpenPopup(("##script_popup" + s->Name()).c_str());
              }
            }
          }
        }

        ImGui::GetStyle().ButtonTextAlign = og_button_txt_align;

        bool clear = false;
        if (ImGui::BeginPopup(("##script_popup" + s->Name()).c_str())) {
          if (clear) {
             
          }

          ImGui::EndPopup();
        }

        ImGui::PopItemWidth();
        // Advance to next column 

        ui::EndPropertyGrid();

        ui::BeginPropertyGrid();
        
        /// display exported properties
        for (auto& [field_id , val] : s->GetFields()) {
          ImGui::PushID(val->name.c_str());
          if (val == nullptr) {
            ImGui::Text("Field [%lld] is corrupt in script %s" , field_id.Get() , s->Name().c_str());
            continue;
          }
          if (val->value.IsArray()) {
          } else {
            bool result = false;

            switch (val->value.Type()) {
              case ValueType::BOOL: 
                
              break;
              case ValueType::CHAR: break;
              case ValueType::INT8: break;
              case ValueType::UINT8: break;
              case ValueType::INT16: break;
              case ValueType::UINT16: break;
              case ValueType::INT32: {
                int32_t v = val->value.Read<int32_t>();
                if (ui::Property(val->name.c_str() , v)) {
                  val->value.Set(v);
                  result = true;
                }
              } break;
              case ValueType::UINT32: break;
              case ValueType::INT64: break;
              case ValueType::UINT64: break;
              case ValueType::FLOAT: break;
              case ValueType::DOUBLE: break;
              case ValueType::VEC2: break;
              case ValueType::VEC3: break;
              case ValueType::VEC4: break;
              case ValueType::STRING: { 
                // std::string v = val->value.Get<std::string>();
                ui::ShiftCursor(10.f , 9.f);
                ImGui::Text("%s" , val->name.c_str());

                ImGui::NextColumn();
                ui::ShiftCursorY(4.f);
                ImGui::PushItemWidth(-1);

                // ImGui::InputText(" > %s" , v.data() , v.length());

                ImGui::PopItemWidth();
                ImGui::NextColumn();
                ui::Underline();

              } break;
              case ValueType::ASSET: break;
              case ValueType::ENTITY: break;
              default:
                ImGui::Text("Field value %s has corrupted type" , val->name.c_str());
            }

          } 
            
          ImGui::PopID();
        }

        ui::EndPropertyGrid();
      }
    });
    
    DrawComponent<Mesh>("Mesh" , [](Entity* selection) {
    });
  }

} // namespace other
