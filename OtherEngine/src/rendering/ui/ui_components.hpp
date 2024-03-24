/**
 * \file rendering/ui/ui_components.hpp
 **/
#ifndef OTHER_ENGINE_UI_COMPONENTS_HPP
#define OTHER_ENGINE_UI_COMPONENTS_HPP

#include <string>
#include <vector>

#include <imgui/imgui.h>

#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_colors.hpp"

namespace other {
namespace ui {

  template <typename T>
  void Table(
    const std::string& name , const std::vector<std::string>& headers , 
    const ImVec2& size , uint32_t columns , T callback
  ) {
    ScopedStyle style(ImGuiStyleVar_CellPadding , ImVec2(4.f , 0.f));
    ImColor bg_color = ImColor(ui::theme::background);
    const ImColor col_row_alt = ui::theme::ColorWithMultiplier(bg_color , 1.2f);
    ScopedColor row_color(ImGuiCol_TableRowBg , bg_color.Value);
    ScopedColor row_alt_color(ImGuiCol_TableRowBgAlt , col_row_alt.Value);
    ScopedColor table_color(ImGuiCol_ChildBg , bg_color.Value);

    ImGuiTableFlags flags = ImGuiTableFlags_NoPadInnerX | 
      ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | 
      ImGuiTableFlags_ScrollY   | ImGuiTableFlags_RowBg;
    if (!ImGui::BeginTable("Project-Table" , columns , flags , size)) {
      return;
    }

    float edge_offset = 4.f;
    const float cursor_x = ImGui::GetCursorScreenPos().x;

    for (uint32_t i = 0; i < columns; ++i) {
      ImGui::TableSetupColumn(headers[i].c_str());
    }

    {
      const ImColor active_color = theme::ColorWithMultiplier(bg_color , 1.2f);
      ScopedColorStack color_stack(ImGuiCol_TableHeaderBg , active_color);

      ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount() , 1);
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers , 22.f);
      
      for (uint32_t i = 0; i < columns; ++i) {
        const char* header = ImGui::TableGetColumnName(i);
        ImGui::PushID(header);
        ShiftCursor(edge_offset * 3.f , edge_offset * 2.f);
        ImGui::TableHeader(header);
        ShiftCursor(-edge_offset * 3.f , -edge_offset * 2.f);
        ImGui::PopID();
      }
      ImGui::SetCursorScreenPos(ImVec2(cursor_x , ImGui::GetCursorScreenPos().y));
      Underline(false , 0.f , 5.f);
    }

    callback();

    ImGui::EndTable();
  }

} // namespace ui
} // namespace other

#endif // !OTHER_ENGINE_UI_COMPONENTS_HPP
