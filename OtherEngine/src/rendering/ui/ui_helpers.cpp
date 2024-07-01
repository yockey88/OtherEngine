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
  
  void DrawTodoReminder(const std::string& item) {
    ScopedColor red(ImGuiCol_Text , ui::theme::red);
    ImGui::Text("TODO: IMPLEMENT %s" , item.c_str());
  }
  
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
  
  bool Checkbox(const char* label , bool* value) {
    bool changed = ImGui::Checkbox(label , value);
    DrawItemActivityOutline();
    return changed;
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
  
  bool Selectable(const std::string& label , bool selected , bool* is_hovered , ImGuiSelectableFlags flags , const ImVec2& size_arg) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) {
      return false;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label.c_str());
    ImVec2 label_size = ImGui::CalcTextSize(label.c_str() , nullptr , true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    ImGui::ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not 
    //  visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_SpanAvailWidth)) {
      size.x = ImMax(label_size.x, max_x - min_x);
    }

    // Text stays at the submission position, but bounding box may be extended on both sides
    const ImVec2 text_min = pos;
    const ImVec2 text_max(min_x + size.x, pos.y + size.y);

    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    ImRect bb(min_x, pos.y, text_max.x, text_max.y);    
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0) {
      const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
      const float spacing_y = style.ItemSpacing.y;
      const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
      const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
      bb.Min.x -= spacing_L;
      bb.Min.y -= spacing_U;
      bb.Max.x += (spacing_x - spacing_L);
      bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
    const float backup_clip_rect_min_x = window->ClipRect.Min.x;
    const float backup_clip_rect_max_x = window->ClipRect.Max.x;
    if (span_all_columns) {
      window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
      window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
    }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const bool item_add = ImGui::ItemAdd(bb, id, NULL, disabled_item ? ImGuiItemFlags_Disabled : ImGuiItemFlags_None);
    if (span_all_columns) {
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }

    if (!item_add) {
      return false;;
    }

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global) { // Only testing this as an optimization
      ImGui::BeginDisabled();
    }

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns) {
      if (g.CurrentTable) {
        ImGui::TablePushBackgroundChannel();
      } else if (window->DC.CurrentColumns) {
        ImGui::PushColumnsBackground();
      }
      g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasClipRect;
      g.LastItemData.ClipRect = window->ClipRect;
    }

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { 
      button_flags |= ImGuiButtonFlags_NoHoldingActiveId; 
    }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner)     { 
      button_flags |= ImGuiButtonFlags_NoSetKeyOwner; 
    }
    if (flags & ImGuiSelectableFlags_SelectOnClick)     { 
      button_flags |= ImGuiButtonFlags_PressedOnClick; 
    }
    if (flags & ImGuiSelectableFlags_SelectOnRelease)   { 
      button_flags |= ImGuiButtonFlags_PressedOnRelease; 
    }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick)  { 
      button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; 
    }
    if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { 
      button_flags |= ImGuiButtonFlags_AllowOverlap; 
    }
    if ((g.LastItemData.InFlags & ImGuiItemFlags_AllowOverlap)) { 
      button_flags |= ImGuiButtonFlags_AllowOverlap; 
    }

    const bool was_selected = selected;
    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Auto-select when moved into
    // - This will be more fully fleshed in the range-select branch
    // - This is not exposed as it won't nicely work with some user side handling of shift/control
    // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
    //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
    //   - (2) usage will fail with clipped items
    //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
    if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId) {
      if (g.NavJustMovedToId == id) {
        selected = pressed = true;
      }
    }

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover))) {
      if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent) {
        ImGui::SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, ImGui::WindowRectAbsToRel(window, bb)); // (bb == NavRect)
        g.NavDisableHighlight = true;
      }
    }

    if (pressed) {
      ImGui::MarkItemEdited(id);
    }

    // In this branch, Selectable() cannot toggle the selection so this will never trigger.
    if (selected != was_selected) { //-V547
      g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;
    }

    // Render
    if (hovered || selected) {
      const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
      ImGui::RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
    }

    if (g.NavId == id) {
      ImGui::RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if (span_all_columns) {
      if (g.CurrentTable) {
        ImGui::TablePopBackgroundChannel();
      } else if (window->DC.CurrentColumns) {
        ImGui::PopColumnsBackground();
      }
    }

    ImGui::RenderTextClipped(text_min, text_max, label.c_str(), nullptr, &label_size, style.SelectableTextAlign, &bb);

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && 
        !(g.LastItemData.InFlags & ImGuiItemFlags_SelectableDontClosePopup)) {
      ImGui::CloseCurrentPopup();
    }

    if (disabled_item && !disabled_global) {
      ImGui::EndDisabled();
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    if (is_hovered != nullptr) {
      *is_hovered = hovered;
    }

    if (pressed || hovered) {
      DrawItemActivityOutline();
    }
    
    return pressed;
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
  
  bool BeginCombo(const char* label , const char* preview_value , ImGuiComboFlags flags) {
    bool opened = ImGui::BeginCombo(label , preview_value , flags);
    DrawItemActivityOutline();
    return opened;
  }

  void EndCombo() {
    ImGui::EndCombo();
  }
  
  bool PropertyDropdown(const char* label , const char** options , int32_t option_count , uint32_t& selected , const char* help_text) {
    const char* current = options[selected];
    ShiftCursor(10.f, 9.f);
    ImGui::Text("%s" , label);

    if (std::strlen(help_text) != 0) {
      ImGui::SameLine();
      HelpMarker(help_text);
    }

    ImGui::NextColumn();
    ShiftCursorY(4.f);
    ImGui::PushItemWidth(-1);

    bool modified = false;

    const std::string id = "##" + std::string{ label };
    if (BeginCombo(id.c_str() , current)) {
      for (uint32_t i = 0; i < option_count; ++i) {
        const bool is_selected = (current == options[i]);
        if (ImGui::Selectable(options[i] , is_selected)) {
          current = options[i];
          selected = i;
          modified = true;
        }

        if (is_selected) {
          ImGui::SetItemDefaultFocus();
        }
      }

      EndCombo();
    }

    ImGui::PopItemWidth();
    ImGui::NextColumn();
    Underline();

    return modified;
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
  
  bool Property(const char* label , bool* value , const char* help_text) {
    bool modified = false;
    BeginProperty(label , help_text);
    modified = Checkbox(fmtstr("##{}" , label).c_str() , value);
    EndProperty();
    return modified;
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
