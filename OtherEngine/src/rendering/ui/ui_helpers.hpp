/**
 * \file rendering/ui/ui_helpers.hpp
 **/
#ifndef OTHER_ENGINE_UI_HELPERS_HPP
#define OTHER_ENGINE_UI_HELPERS_HPP

#include <cstdint>
#include <utility>
#include <string>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "rendering/ui/ui_colors.hpp"

#define UI_COLOR(r , g , b , a) ImVec4(r / 255.0f , g / 255.0f , b / 255.0f , a / 255.0f)

namespace other {

  class ScopedID {
    public:
      ScopedID(const ScopedID&) = delete;
      ScopedID& operator=(const ScopedID&) = delete;
  
      template<typename T>
      ScopedID(T id) { 
        ImGui::PushID(id); 
      }

      ~ScopedID() { 
        ImGui::PopID(); 
      }
  };
  
  class ScopedFont {
    bool pushed = false;
  
    public:
      ScopedFont(ImFont* font);
      ~ScopedFont();
  };
  
  class ScopedStyle {        
    bool pushed = false;
  
    public:
      ScopedStyle(ImGuiStyleVar var , float new_val);
      ScopedStyle(ImGuiStyleVar var , const ImVec2& new_col);
      ~ScopedStyle(); 
  };
  
  class ScopedColor {
    public:
      ScopedColor(ImGuiCol col , uint32_t new_col);
      ScopedColor(ImGuiCol col , const ImVec4& new_col);
      ~ScopedColor();
  };
  
  class ScopedColorStack {
    uint32_t count;
  
    template <typename ColorType , typename... Colors>
    void PushColor(ImGuiCol col , ColorType color , Colors&&... colors) {
      if constexpr (sizeof... (colors) == 0) {
        ImGui::PushStyleColor(col , ImColor(color).Value);
      } else {
        ImGui::PushStyleColor(col , ImColor(color).Value);
        PushColor(std::forward<Colors>(colors)...);
      }
    }
  
    public:
      ScopedColorStack(const ScopedColorStack&) = delete;
      ScopedColorStack& operator=(const ScopedColorStack&) = delete;
  
      template <typename ColorType , typename... Colors>
      ScopedColorStack(ImGuiCol first_color , ColorType first , Colors&&... colors) 
          : count((sizeof... (colors) / 2) + 1) {
        static_assert(
          (sizeof... (colors) & 1u) == 0 ,
          "ScopedColorStack requires an even number of arguments"
        );
  
        PushColor(first_color , first , std::forward<Colors>(colors)...);
      }
  
      ~ScopedColorStack();
  };

namespace ui {

  typedef int32_t OutlineFlags;
  enum OutlineFlags_ {
    OutlineFlags_None            = 0 ,
    OutlineFlags_WhenHovered     = 1 << 1 ,
    OutlineFlags_WhenActive      = 1 << 2 ,
    OutlineFlags_WhenInactive    = 1 << 3 ,
    OutlineFlags_HighlightActive = 1 << 4,
    OutlineFlags_NoHighlightActive = OutlineFlags_WhenHovered | OutlineFlags_WhenActive | OutlineFlags_WhenInactive ,
    OutlineFlags_NoOutlineInactive = OutlineFlags_WhenHovered | OutlineFlags_WhenActive | OutlineFlags_HighlightActive ,
    OutlineFlags_All = OutlineFlags_WhenHovered | OutlineFlags_WhenActive | OutlineFlags_WhenInactive | OutlineFlags_HighlightActive ,
  };

  bool IsItemDisabled();
  ImRect GetItemRect();

  void DrawItemActivityOutline(OutlineFlags flags = OutlineFlags_All , ImColor color_highlight = theme::accent , float rounding = GImGui->Style.FrameRounding);

  inline ImRect RectExpanded(const ImRect& rect , float x , float y) {
    return ImRect(
      ImVec2(rect.Min.x - x , rect.Min.y - y) ,
      ImVec2(rect.Max.x + x , rect.Max.y + y)
    );
  }

  void PushId();
  void PopId();

  void ShiftCursor(float x , float y);
  void ShiftCursorX(float x);
  void ShiftCursorY(float y);
  void Underline(bool full_width = false , float offx = 0.f , float offy = -1.f);
  
  bool BeginTreeNode(const char* name, bool default_open);    
  void EndTreeNode();    
  
  bool ColoredButton(const char* label, const ImVec4& backgroundColor, ImVec2 buttonSize);    
  bool ColoredButton(
    const char* label , const ImVec4& background , 
    const ImVec4& foreground , ImVec2 size
  );    

  /// this is a wrapper around ImGui::DragFloat so we can add the outline using other::ui::DrawItemActivityOutline(...) 
  ///   after the dragger is drawn
  bool DragFloat(const char* label , float* v , float v_speed = 1.f , float v_min = 0.f , float v_max = 0.f , 
                 const char* format = "%.3f" , ImGuiSliderFlags flags = 0);
  
  bool TableRowClickable(const char* id, float rowHeight);
  
  void Separator(ImVec2 size, ImVec4 color);
  
  bool IsWindowFocused(const char* window_name, const bool check_root_window);
  
  void HelpMarker(const char* desc);
  
  void BeginPropertyGrid(
    uint32_t columns , 
    const ImVec2& item_spacing = ImVec2(8.f , 8.f) , 
    const ImVec2& frame_pading = ImVec2(4.f , 4.f)
  );
  
  void EndPropertyGrid();

  bool OpenPopup(const std::string& str_id , ImGuiWindowFlags flags = 0);
  void EndPopup();

} // namespace ui
} // namespace other

#endif // !OTHER_ENGINE_UI_HELPERS_HPP
