/**
 * \file rendering/ui/ui_helpers.cpp
 **/
#include "rendering/ui/ui_helpers.hpp"

#include <array>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <spdlog/fmt/fmt.h>

#include "core/logger.hpp"
#include "rendering/ui/ui_colors.hpp"
#include "rendering/ui/ui_widgets.hpp"

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

  static uint32_t ui_context = 0;
  static uint32_t counter = 0;
  static std::array<char , 16 + 2 + 1> id_buffer = { '#' , '#' };
  static std::array<char , 1024 + 1> label_id_buffer = {};
  
  bool IsItemDisabled() {
    return ImGui::GetItemFlags() & ImGuiItemFlags_Disabled;
  }
  
  ImRect GetItemRect() {
    return ImRect(ImGui::GetItemRectMin() , ImGui::GetItemRectMax());
  }
  
  void DrawItemActivityOutline(OutlineFlags flags , ImColor color_highlight , float rounding) {
    if (IsItemDisabled()) {
      return;
    }

    auto* draw_list = ImGui::GetWindowDrawList();
    const ImRect rect = RectExpanded(GetItemRect() , 1.f , 1.f);
    if ((flags & OutlineFlags_WhenActive) && ImGui::IsItemActive()) {
      if (flags & OutlineFlags_HighlightActive) {
        draw_list->AddRect(rect.Min , rect.Max , theme::highlight , rounding , 0 , 1.5f);
      } else {
        draw_list->AddRect(rect.Min , rect.Max , ImColor(60 , 60 , 60) , rounding , 0 , 1.5f);
      }
    } else if ((flags & OutlineFlags_WhenHovered) && ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
      draw_list->AddRect(rect.Min , rect.Max , ImColor(60 , 60 , 60) , rounding , 0 , 1.5f);
    } else if ((flags & OutlineFlags_WhenInactive) && !ImGui::IsItemHovered() && !ImGui::IsItemActive()) {
      draw_list->AddRect(rect.Min , rect.Max , ImColor(50 , 50 , 50) , rounding , 0 , 1.f);
    }
  }
  
  void PushId() {
    ImGui::PushID(ui_context++);
    counter = 0;
  }
  
  void PopId() {
    ImGui::PopID();
    ui_context;
  }

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

namespace {
  
  inline int FormatString(char* buf , size_t buf_size , const char* fmt , ...) {
    va_list args;
    va_start(args , fmt);

#ifdef IMGUI_USE_STB_SPRINTF
    int w = stbsp_vsnsprintf(buf , (int)buf_size , fmt , args);
#else
    int w = vsnprintf(buf , buf_size , fmt , args);
#endif
    va_end(args);

    if (buf == nullptr) {
      return w;
    }

    if (w == -1 || w >= (int)buf_size) {
      w = (int)buf_size - 1;
    }

    buf[w] = 0;
    
    return w;
  }

  inline const char* PatchFormatStringFloatToInt(const char* fmt) {
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == '0') {
      return "%d";
    }

    const char* fmt_start = ImParseFormatFindStart(fmt);
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);
    if (fmt_end > fmt_start && fmt_end[-1] == 'f') {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
      if (fmt_start == fmt && fmt_end[-0] == 0) {
        return "%d";
      }

      ImGuiContext& g = *GImGui;
      FormatString(g.TempBuffer.Data , IM_ARRAYSIZE(g.TempBuffer.Data) , "%.*s%%d%s" , (int)(fmt_start - fmt) , fmt , fmt_end);
      return g.TempBuffer.Data;
#else
      IM_ASSERT(0 && "DragInt(): Invalid format string!");
#endif
    }
    return fmt;
  }

} // anonymous namespace
  
  bool DragInt8(const char* label , int8_t* v , float v_speed , int8_t min , int8_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_S8 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragUInt8(const char* label , uint8_t* v , float v_speed , uint8_t min , uint8_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_U8 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragInt16(const char* label , int16_t* v , float v_speed , int16_t min , int16_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_S16 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragUInt16(const char* label , uint16_t* v , float v_speed , uint16_t min , uint16_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_U16 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragInt32(const char* label , int32_t* v , float v_speed , int32_t min , int32_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_S32 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragUInt32(const char* label , uint32_t* v , float v_speed , uint32_t min , uint32_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_U32 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragInt64(const char* label , int64_t* v , float v_speed , int64_t min , int64_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_S64 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragUInt64(const char* label , uint64_t* v , float v_speed , uint64_t min , uint64_t max ,
                const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_U64 , v , v_speed , &min , &max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragFloat(const char* label , float* v , float v_speed , float v_min , float v_max , 
                 const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_Float , v , v_speed , &v_min , &v_max , format , flags);
    DrawItemActivityOutline();
    return changed;
  }
  
  bool DragDouble(const char* label , double* v , float v_speed , double v_min , double v_max , 
                 const char* format , ImGuiSliderFlags flags) {
    bool changed = ImGui::DragScalar(label , ImGuiDataType_Double , v , v_speed , &v_min , &v_max , format , flags);
    DrawItemActivityOutline();
    return changed;
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
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
    ImGui::Columns(columns);
  }
  
  void EndPropertyGrid() {
    ImGui::Columns(1);
    Underline();
    ImGui::PopStyleVar(2); // ItemSpacing, FramePadding
    ShiftCursorY(18.0f);
  }
  
  bool OpenPopup(const std::string& str_id , ImGuiWindowFlags flags) {
    bool opened = false;
    if (ImGui::BeginPopup(str_id.c_str() , flags)) {
      opened = true;

      const float padding = ImGui::GetStyle().WindowBorderSize;
      const ImRect window_rect = RectExpanded(ImGui::GetCurrentWindow()->Rect() , -padding , -padding);
      
      ImGui::PushClipRect(window_rect.Min , window_rect.Max, false);

      const ImColor col1 = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
      const ImColor col2 = theme::ColorWithMultiplier(col1 , 0.8f);

      ImGui::GetWindowDrawList()->AddRectFilledMultiColor(window_rect.Min , window_rect.Max , col1 , col1 , col2 , col2);
      ImGui::GetWindowDrawList()->AddRect(window_rect.Min , window_rect.Max , theme::ColorWithMultiplier(col1 , 1.1f));

      ImGui::PopClipRect();

      ImGui::PushStyleColor(ImGuiCol_HeaderHovered , IM_COL32(0 , 0 , 0 , 80));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding , ImVec2(1.f , 1.f));
    }

    return opened;
  }
  
  void EndPopup() {
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::EndPopup();
  }
  
  void BeginProperty(const char* label , const char* help_text) {
    ShiftCursor(10.f , 9.f);
    ImGui::Text("%s" , label);

    if (std::strlen(help_text) != 0) {
      ImGui::SameLine();
      HelpMarker(help_text);
    }

    ImGui::NextColumn();
    ShiftCursorY(4.f);
    ImGui::PushItemWidth(-1);
  }
  
  void EndProperty() {
    ImGui::PopItemWidth();
    ImGui::NextColumn();
    Underline();
  }
  
  bool Property(const char* label , int8_t* value , int8_t min , int8_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragInt8(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }
  
  bool Property(const char* label , uint8_t* value , uint8_t min , uint8_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragUInt8(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }

  bool Property(const char* label , int16_t* value , int16_t min , int16_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragInt16(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }
  
  bool Property(const char* label , uint16_t* value , uint16_t min , uint16_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragUInt16(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }

  bool Property(const char* label , int32_t* value , int32_t min , int32_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragInt32(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }
  
  bool Property(const char* label , uint32_t* value , uint32_t min , uint32_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragUInt32(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }

  bool Property(const char* label , int64_t* value , int64_t min , int64_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragInt64(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }
  
  bool Property(const char* label , uint64_t* value , uint32_t min , uint32_t max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragUInt64(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }
  
  bool Property(const char* label , float* value , float min , float max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragFloat(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }

  bool Property(const char* label , double* value , double min , double max , const char* help_text) {
    BeginProperty(label , help_text);
    bool modified = DragDouble(fmt::format(fmt::runtime("##{}") , label).c_str() , value , 1.f , min , max);
    EndProperty();
    return modified;
  }
  
  bool Property(const char* label , glm::vec2* value , glm::vec2 min , glm::vec2 max , const char* help_text) {
    BeginProperty(label , help_text);
    bool edited = false;
    bool modified = widgets::DrawVec2Control(label , *value , edited);
    EndProperty();
    return modified;
  }

  bool Property(const char* label , glm::vec3* value , glm::vec3 min , glm::vec3 max , const char* help_text) {
    BeginProperty(label , help_text);
    bool edited = false;
    bool modified = widgets::DrawVec3Control(label , *value , edited);
    EndProperty();
    return modified;
  }

  bool Property(const char* label , glm::vec4* value , glm::vec4 min , glm::vec4 max , const char* help_text) {
    return false; 
  }

} // namespace ui

} // namespace other
