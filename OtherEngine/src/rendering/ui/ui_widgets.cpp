/**
 * \file rendering/ui/ui_widgets.cpp
 **/
#include "rendering/ui/ui_widgets.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "core/logger.hpp"
#include "input/keyboard.hpp"
#include "rendering/ui/ui_helpers.hpp"

namespace other {
namespace ui {
namespace widgets {
  
  bool DrawVec2Control(const std::string_view label , glm::vec2& value , bool& edited , 
                       float reset_val , float col_w , VectorAxis axes) {
    bool modified = false;
    
    ImGui::TableSetColumnIndex(0);
    ShiftCursor(17.f , 7.f);

    ImGui::Text("%s" , label.data());
    Underline(false , 0.f , 2.f);

    ImGui::TableSetColumnIndex(1);
    ShiftCursor(7.f , 0.f);

    modified = EditVec2(label , ImVec2(ImGui::GetContentRegionAvail().x - 8.f , ImGui::GetFrameHeightWithSpacing() + 8.f) , 
                        reset_val , edited , value , axes);

    return modified;
  }
  
  bool EditVec2(const std::string_view label , ImVec2 size , float reset_val , bool& edited , glm::vec2& value ,
                VectorAxis axes , float speed , const glm::vec2& v_min , 
                const glm::vec2& v_max , const char* format , ImGuiSliderFlags flags) {
    ImGui::BeginVertical((std::string(label) + "fr").data());
    bool changed = false;
    {
      const float spacing = 8.f;
      ScopedStyle item_spacing(ImGuiStyleVar_ItemSpacing , ImVec2(spacing , 0.f));
      ScopedStyle padding(ImGuiStyleVar_WindowPadding , ImVec2(0.f , 0.f));

      const float frame_padding = 2.f;
      const float outline_spacing = 1.f;
      const float line_height = GImGui->Font->FontSize + frame_padding * 2.f;
      const ImVec2 button_size = {
        line_height + 2.f ,
        line_height
      };

      const float input_item_w = size.x / 3.f - button_size.x;

      ShiftCursorY(frame_padding);

      const ImGuiIO& io = ImGui::GetIO();
      auto bold_font = io.Fonts->Fonts[0];

      auto draw_control 
        = [&](const std::string& label , float& value , const ImVec4& colorn , const ImVec4& colorh , const ImVec4& colorp , 
              bool multi_select , float speed , float v_min , float v_max , const char* format , ImGuiSliderFlags flags) {
          {
            ScopedStyle button_frame(ImGuiStyleVar_FramePadding , ImVec2(frame_padding , 0.f));
            ScopedStyle button_rounding(ImGuiStyleVar_FrameRounding , 1.f);
            ScopedColorStack button_colors(
              ImGuiCol_Button , colorn , 
              ImGuiCol_ButtonHovered , colorh , 
              ImGuiCol_ButtonActive , colorp
            );

            ScopedFont font(bold_font);

            ShiftCursorY(frame_padding / 2.f);
            if (ImGui::Button(label.c_str() , button_size)) {
              value = reset_val;
              changed = true;
            }
          }

          ImGui::SameLine(0.f , outline_spacing);
          ImGui::SetNextItemWidth(input_item_w);

          ShiftCursorY(-frame_padding / 2.f);

          ImGui::PushItemFlag(ImGuiItemFlags_MixedValue , multi_select);

          bool was_temp_input_active = ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str()));
          changed |= DragFloat(("##" + label).c_str() , &value , speed , v_min , v_max , format , flags);

          if (changed && Keyboard::Pressed(Keyboard::Key::OE_TAB)) {
            edited = true;
          }

          if (ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str()))) {
            changed = false;
          }

          ImGui::PopItemFlag();

          if (was_temp_input_active) {
            edited |= ImGui::IsItemDeactivatedAfterEdit();
          }
        };

      draw_control(
        "X" , value.x , 
        ImVec4{ 0.8f , 0.1f , 0.15f , 1.f } , 
        ImVec4{ 0.9f , 0.2f , 0.2f , 1.f } , 
        ImVec4{ 0.8f , 0.1f , 0.15f , 1.f } , 
        (axes & VectorAxis::X) , 
        speed, v_min.x , v_max.x , format , flags
      );

      ImGui::SameLine(0.f , outline_spacing);
      
      draw_control(
        "Y" , value.y , 
        ImVec4{ 0.2f , 0.7f , 0.2f , 1.f } , 
        ImVec4{ 0.3f , 0.8f , 0.3f , 1.f } , 
        ImVec4{ 0.2f , 0.7f , 0.2f , 1.f } , 
        (axes & VectorAxis::Y) , 
        speed, v_min.y , v_max.y , format , flags
      );
      
      ImGui::SameLine(0.f , outline_spacing);

      ImGui::EndVertical();

    }
    return changed || edited;
  }

  bool DrawVec3Control(const std::string_view label , glm::vec3& value , bool& edited , 
                       float reset_val , float col_w , VectorAxis axes) {
    bool modified = false;
    
    ImGui::TableSetColumnIndex(0);
    ShiftCursor(17.f , 7.f);

    ImGui::Text("%s" , label.data());
    Underline(false , 0.f , 2.f);

    ImGui::TableSetColumnIndex(1);
    ShiftCursor(7.f , 0.f);

    modified = EditVec3(label , ImVec2(ImGui::GetContentRegionAvail().x - 8.f , ImGui::GetFrameHeightWithSpacing() + 8.f) , 
                        reset_val , edited , value , axes);

    return modified;
  }
  
  bool EditVec3(const std::string_view label , ImVec2 size , float reset_val , bool& edited , glm::vec3& value ,
                VectorAxis axes , float speed , const glm::vec3& v_min , 
                const glm::vec3& v_max , const char*  format , ImGuiSliderFlags flags) {
    ImGui::BeginVertical((std::string(label) + "fr").data());
    bool changed = false;
    {
      const float spacing = 8.f;
      ScopedStyle item_spacing(ImGuiStyleVar_ItemSpacing , ImVec2(spacing , 0.f));
      ScopedStyle padding(ImGuiStyleVar_WindowPadding , ImVec2(0.f , 0.f));

      const float frame_padding = 2.f;
      const float outline_spacing = 1.f;
      const float line_height = GImGui->Font->FontSize + frame_padding * 2.f;
      const ImVec2 button_size = {
        line_height + 2.f ,
        line_height
      };

      const float input_item_w = size.x / 3.f - button_size.x;

      ShiftCursorY(frame_padding);

      const ImGuiIO& io = ImGui::GetIO();
      auto bold_font = io.Fonts->Fonts[0];

      auto draw_control 
        = [&](const std::string& label , float& value , const ImVec4& colorn , const ImVec4& colorh , const ImVec4& colorp , 
              bool multi_select , float speed , float v_min , float v_max , const char* format , ImGuiSliderFlags flags) {
          {
            ScopedStyle button_frame(ImGuiStyleVar_FramePadding , ImVec2(frame_padding , 0.f));
            ScopedStyle button_rounding(ImGuiStyleVar_FrameRounding , 1.f);
            ScopedColorStack button_colors(
              ImGuiCol_Button , colorn , 
              ImGuiCol_ButtonHovered , colorh , 
              ImGuiCol_ButtonActive , colorp
            );

            ScopedFont font(bold_font);

            ShiftCursorY(frame_padding / 2.f);
            if (ImGui::Button(label.c_str() , button_size)) {
              value = reset_val;
              changed = true;
            }
          }

          ImGui::SameLine(0.f , outline_spacing);
          ImGui::SetNextItemWidth(input_item_w);

          ShiftCursorY(-frame_padding / 2.f);

          ImGui::PushItemFlag(ImGuiItemFlags_MixedValue , multi_select);

          bool was_temp_input_active = ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str()));
          changed |= DragFloat(("##" + label).c_str() , &value , speed , v_min , v_max , format , flags);

          if (changed && Keyboard::Pressed(Keyboard::Key::OE_TAB)) {
            edited = true;
          }

          if (ImGui::TempInputIsActive(ImGui::GetID(("##" + label).c_str()))) {
            changed = false;
          }

          ImGui::PopItemFlag();

          if (was_temp_input_active) {
            edited |= ImGui::IsItemDeactivatedAfterEdit();
          }
        };

      draw_control(
        "X" , value.x , 
        ImVec4{ 0.8f , 0.1f , 0.15f , 1.f } , 
        ImVec4{ 0.9f , 0.2f , 0.2f , 1.f } , 
        ImVec4{ 0.8f , 0.1f , 0.15f , 1.f } , 
        (axes & VectorAxis::X) , 
        speed, v_min.x , v_max.x , format , flags
      );

      ImGui::SameLine(0.f , outline_spacing);
      
      draw_control(
        "Y" , value.y , 
        ImVec4{ 0.2f , 0.7f , 0.2f , 1.f } , 
        ImVec4{ 0.3f , 0.8f , 0.3f , 1.f } , 
        ImVec4{ 0.2f , 0.7f , 0.2f , 1.f } , 
        (axes & VectorAxis::Y) , 
        speed, v_min.y , v_max.y , format , flags
      );
      
      ImGui::SameLine(0.f , outline_spacing);
      
      draw_control(
        "Z" , value.z , 
        ImVec4{ 0.1f , 0.25f , 0.8f , 1.f } , 
        ImVec4{ 0.2f , 0.35f , 0.9f , 1.f } , 
        ImVec4{ 0.1f , 0.25f , 0.8f , 1.f } , 
        (axes & VectorAxis::Z) , 
        speed, v_min.z , v_max.z , format , flags
      );

      ImGui::EndVertical();

    }
    return changed || edited;
  }

} // namespace widgets
} // namespace ui
} // namespace other
