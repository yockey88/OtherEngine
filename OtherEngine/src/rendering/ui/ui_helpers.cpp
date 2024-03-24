/**
 * \file rendering/ui/ui_helpers.cpp
 **/
#include "rendering/ui/ui_helpers.hpp"

namespace other {

  ScopedFont::ScopedFont(ImFont* font) {
    if (font != nullptr) {
      ImGui::PushFont(font);
      pushed = true;
    }
  }

  ScopedFont::~ScopedFont() {
    if (pushed) {
      ImGui::PopFont();
    }
  }
  
  ScopedStyle::ScopedStyle(ImGuiStyleVar var , float new_val) {
    const ImGuiDataVarInfo* var_info = ImGui::GetStyleVarInfo(var);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1) {
      ImGui::PushStyleVar(var , new_val);
      pushed = true;
    }
  }

  ScopedStyle::ScopedStyle(ImGuiStyleVar var , const ImVec2& new_col) {
    const ImGuiDataVarInfo* var_info = ImGui::GetStyleVarInfo(var);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 2) {
      ImGui::PushStyleVar(var , new_col);
      pushed = true;
    }
  }

  ScopedStyle::~ScopedStyle() {
    if (pushed) {
      ImGui::PopStyleVar();
    }
  }
  
  ScopedColor::ScopedColor(ImGuiCol col , uint32_t new_col) {
    ImGui::PushStyleColor(col , new_col);
  }

  ScopedColor::ScopedColor(ImGuiCol col , const ImVec4& new_col) {
    ImGui::PushStyleColor(col , new_col);
  }

  ScopedColor::~ScopedColor() {
    ImGui::PopStyleColor();
  }

  ScopedColorStack::~ScopedColorStack() {    
    ImGui::PopStyleColor(count);
  }

namespace ui {

  void ShiftCursor(float x , float y) {
    const ImVec2 cursor_pos = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor_pos.x + x , cursor_pos.y + y));
  }
  
  void ShiftCursorX(float x) {
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x);
  }
  
  void ShiftCursorY(float y) {
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y);
  }
  
  void Underline(bool full_width , float offx , float offy) {
    if (full_width) {
      if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)  {
        ImGui::PushColumnsBackground();
      } else if (ImGui::GetCurrentTable() != nullptr) {
        ImGui::TablePushBackgroundChannel();
      }
    }
  
    const float width = full_width ? 
      ImGui::GetWindowWidth() : 
      ImGui::GetContentRegionAvail().x;
    
    const ImVec2 cursor = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
      ImVec2(cursor.x  + offx , cursor.y + offy),
      ImVec2(cursor.x + width , cursor.y + offy),
      IM_COL32(26, 26, 26, 255) /* dark background */, 1.0f
    );
  
    if (full_width) {
      if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr) {
        ImGui::PopColumnsBackground();
      } else if (ImGui::GetCurrentTable() != nullptr) {
        ImGui::TablePopBackgroundChannel();
      }
    }
  }
  
  bool BeginTreeNode(const char* name, bool default_open) {
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | 
                                    ImGuiTreeNodeFlags_FramePadding;
    if (default_open) {
      node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }
    
    return ImGui::TreeNodeEx(name , node_flags);
  }
  
  void EndTreeNode() {
    ImGui::TreePop();
  }
  
  bool ColoredButton(const char* label, const ImVec4& backgroundColor, ImVec2 buttonSize) {
    ScopedColor buttonColor(ImGuiCol_Button, backgroundColor);
    return ImGui::Button(label , buttonSize);
  }
  
  bool ColoredButton(
    const char* label , const ImVec4& background , 
    const ImVec4& foreground , ImVec2 size
  ) {
    ScopedColor tcolor(ImGuiCol_Text, foreground);
    ScopedColor bcolor(ImGuiCol_Button, background);
    return ImGui::Button(label , size);
  }
  
  bool TableRowClickable(const char* id, float rowHeight) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    window->DC.CurrLineSize.y = rowHeight;
    
    ImGui::TableNextRow(0, rowHeight);
    ImGui::TableNextColumn();
    
    window->DC.CurrLineTextBaseOffset = 3.0f;
    const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
    const ImVec2 rowAreaMax = { 
      ImGui::TableGetCellBgRect(
        ImGui::GetCurrentTable() , 
        ImGui::TableGetColumnCount() - 1
      ).Max.x , 
      rowAreaMin.y + rowHeight 
    };
  
    ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
    
    bool isRowHovered, held;
    bool isRowClicked = ImGui::ButtonBehavior(
    ImRect(rowAreaMin , rowAreaMax) , ImGui::GetID(id) ,
      &isRowHovered , &held , ImGuiButtonFlags_AllowOverlap
    );
  
    ImGui::SetItemAllowOverlap();
    ImGui::PopClipRect();
  
    return isRowClicked;
  }
  
  void Separator(ImVec2 size, ImVec4 color) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, color);
    ImGui::BeginChild("sep", size);
    ImGui::EndChild();
    ImGui::PopStyleColor();
  }
  
  bool IsWindowFocused(const char* window_name, const bool check_root_window) {
    ImGuiWindow* curr_nav_win = GImGui->NavWindow;
    
    if (check_root_window) {
      // Get the actual nav window (not e.g a table)
      ImGuiWindow* lastWindow = NULL;
      while (lastWindow != curr_nav_win){
        lastWindow = curr_nav_win;
        curr_nav_win = curr_nav_win->RootWindow;
      }
    }
    
    return curr_nav_win == ImGui::FindWindowByName(window_name);
  }
  
  void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
      ImGui::TextUnformatted(desc);
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  }
  
  void BeginPropertyGrid(
    uint32_t columns , 
    const ImVec2& item_spacing , 
    const ImVec2& frame_pading
  ) {
    // PushID();
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
    ImGui::Columns(columns);
  }
  
  void EndPropertyGrid() {
    ImGui::Columns(1);
    Underline();
    ImGui::PopStyleVar(2); // ItemSpacing, FramePadding
    ShiftCursorY(18.0f);
    //PopID();
  }

} // namespace ui

} // namespace other
