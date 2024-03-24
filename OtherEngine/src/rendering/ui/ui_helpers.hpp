/**
 * \file rendering/ui/ui_helpers.hpp
 **/
#ifndef OTHER_ENGINE_UI_HELPERS_HPP
#define OTHER_ENGINE_UI_HELPERS_HPP

#include <cstdint>
#include <utility>

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

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

} // namespace ui
} // namespace other

#endif // !OTHER_ENGINE_UI_HELPERS_HPP
