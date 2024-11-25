/**
 * \file rendering/ui/ui.hpp
 **/
#ifndef OTHER_ENGINE_UI_HPP
#define OTHER_ENGINE_UI_HPP

#include <imgui/imgui.h>

#include "core/UUID.hpp"
#include "core/config.hpp"
#include "core/logger.hpp"

#include "rendering/ui/ui_window.hpp"
#include "rendering/window.hpp"

namespace other {

  enum UIThemeIndex {
    TEXT = ImGuiCol_Text,
    TEXT_DISABLED = ImGuiCol_TextDisabled,
    WINDOW_BG = ImGuiCol_WindowBg,
    CHILD_BG = ImGuiCol_ChildBg,
    POPUP_BG = ImGuiCol_PopupBg,
    BORDER = ImGuiCol_Border,
    BORDER_SHADOW = ImGuiCol_BorderShadow,
    FRAME_BG = ImGuiCol_FrameBg,
    FRAME_BG_HOVERED = ImGuiCol_FrameBgHovered,
    FRAME_BG_ACTIVE = ImGuiCol_FrameBgActive,
    TITLE_BG = ImGuiCol_TitleBg,
    TITLE_BG_ACTIVE = ImGuiCol_TitleBgActive,
    TITLE_BG_COLLAPSED = ImGuiCol_TitleBgCollapsed,
    MENU_BAR_BG = ImGuiCol_MenuBarBg,
    SCROLLBAR_BG = ImGuiCol_ScrollbarBg,
    SCROLLBAR_GRAB = ImGuiCol_ScrollbarGrab,
    SCROLLBAR_GRAB_HOVERED = ImGuiCol_ScrollbarGrabHovered,
    SCROLLBAR_GRAB_ACTIVE = ImGuiCol_ScrollbarGrabActive,
    CHECK_MARK = ImGuiCol_CheckMark,
    SLIDER_GRAB = ImGuiCol_SliderGrab,
    SLIDER_GRAB_ACTIVE = ImGuiCol_SliderGrabActive,
    BUTTON = ImGuiCol_Button,
    BUTTON_HOVERED = ImGuiCol_ButtonHovered,
    BUTTON_ACTIVE = ImGuiCol_ButtonActive,
    HEADER = ImGuiCol_Header,
    HEADER_HOVERED = ImGuiCol_HeaderHovered,
    HEADER_ACTIVE = ImGuiCol_HeaderActive,
    SEPARATOR = ImGuiCol_Separator,
    SEPARATOR_HOVERED = ImGuiCol_SeparatorHovered,
    SEPARATOR_ACTIVE = ImGuiCol_SeparatorActive,
    RESIZE_GRIP = ImGuiCol_ResizeGrip,
    RESIZE_GRIP_HOVERED = ImGuiCol_ResizeGripHovered,
    RESIZE_GRIP_ACTIVE = ImGuiCol_ResizeGripActive,
    TAB = ImGuiCol_Tab,
    TAB_HOVERED = ImGuiCol_TabHovered,
    TAB_ACTIVE = ImGuiCol_TabActive,
    TAB_UNFOCUSED = ImGuiCol_TabUnfocused,
    TAB_UNFOCUSED_ACTIVE = ImGuiCol_TabUnfocusedActive,
    DOCKING_PREVIEW = ImGuiCol_DockingPreview,
    DOCKING_EMPTY_BG = ImGuiCol_DockingEmptyBg,
    PLOT_LINES = ImGuiCol_PlotLines,
    PLOT_LINES_HOVERED = ImGuiCol_PlotLinesHovered,
    PLOT_HISTOGRAM = ImGuiCol_PlotHistogram,
    PLOT_HISTOGRAM_HOVERED = ImGuiCol_PlotHistogramHovered,
    TABLE_HEADER_BG = ImGuiCol_TableHeaderBg,
    TABLE_BORDER_STRONG = ImGuiCol_TableBorderStrong,
    TABLE_BORDER_LIGHT = ImGuiCol_TableBorderLight,
    TABLE_ROW_BG = ImGuiCol_TableRowBg,
    TABLE_ROW_BG_ALT = ImGuiCol_TableRowBgAlt,
    TEXT_SELECTED_BG = ImGuiCol_TextSelectedBg,
    DRAG_DROP_TARGET = ImGuiCol_DragDropTarget,
    NAV_HIGHLIGHT = ImGuiCol_NavHighlight,
    NAV_WINDOWING_HIGHLIGHT = ImGuiCol_NavWindowingHighlight,
    NAV_WINDOWING_DIM_BG = ImGuiCol_NavWindowingDimBg,
    MODAL_WINDOW_DIM_BG = ImGuiCol_ModalWindowDimBg,

    UI_THEME_INDEX_COUNT,
    INVALID_INDEX = UI_THEME_INDEX_COUNT
  };

  constexpr static uint32_t kUiThemeIndexCount = UI_THEME_INDEX_COUNT + 1;
  constexpr static std::array<std::string_view, kUiThemeIndexCount> kThemeIndexStrings = {
    "TEXT", "TEXT_DISABLED",
    "WINDOW_BG", "CHILD_BG", "POPUP_BG",
    "BORDER", "BORDER_SHADOW",
    "FRAME_BG", "FRAME_BG_HOVERED", "FRAME_BG_ACTIVE",
    "TITLE_BG", "TITLE_BG_ACTIVE", "TITLE_BG_COLLAPSED",
    "MENU_BAR_BG",
    "SCROLLBAR_BG", "SCROLLBAR_GRAB", "SCROLLBAR_GRAB_HOVERED", "SCROLLBAR_GRAB_ACTIVE",
    "CHECK_MARK",
    "SLIDER_GRAB", "SLIDER_GRAB_ACTIVE",
    "BUTTON", "BUTTON_HOVERED", "BUTTON_ACTIVE",
    "HEADER", "HEADER_HOVERED", "HEADER_ACTIVE",
    "SEPARATOR", "SEPARATOR_HOVERED", "SEPARATOR_ACTIVE",
    "RESIZE_GRIP", "RESIZE_GRIP_HOVERED", "RESIZE_GRIP_ACTIVE",
    "TAB", "TAB_HOVERED", "TAB_ACTIVE", "TAB_UNFOCUSED", "TAB_UNFOCUSED_ACTIVE",
    "DOCKING_PREVIEW", "DOCKING_EMPTY_BG",
    "PLOT_LINES", "PLOT_LINES_HOVERED", "PLOT_HISTOGRAM", "PLOT_HISTOGRAM_HOVERED",
    "TABLE_HEADER_BG", "TABLE_BORDER_STRONG", "TABLE_BORDER_LIGHT", "TABLE_ROW_BG", "TABLE_ROW_BG_ALT",
    "TEXT_SELECTED_BG",
    "DRAG_DROP_TARGET",
    "NAV_HIGHLIGHT", "NAV_WINDOWING_HIGHLIGHT", "NAV_WINDOWING_DIM_BG",
    "MODAL_WINDOW_DIM_BG",

    "INVALID_INDEX"
  };

  struct UITheme {
    std::string name;
    ImVec4 colors[UI_THEME_INDEX_COUNT];

    UITheme();
    UITheme(const std::string& name, ImVec4 colors[UI_THEME_INDEX_COUNT]);
  };

  /// this seems very strange? why did I write this?
  /// TODO: remove soon
  struct WindowRef {
    const Scope<Window>& main_window;

    WindowRef(const Scope<Window>& window)
        : main_window(window) {}
  };

  class UI {
   public:
    static void Initialize(const ConfigTable& config, const Scope<Window>& window);

    static void SetTheme(const UITheme& theme);

    static bool Enabled();

    template <ui_type T, typename... Args>
    static bool AddWindow(const std::string& title, Args&&... args) {
      UUID id = FNV(title);
      if (ui_windows.find(id) != ui_windows.end()) {
        OE_WARN("UI window [{}] already exists", title);
        return false;
      }

      auto& window = ui_windows[id] = NewScope<T>(title, std::forward<Args>(args)...);
      window->OnAttach();

      return true;
    }

    template <ui_type T>
    static bool AddWindow(const std::string& title, Scope<T>& window) {
      UUID id = FNV(title);
      if (ui_windows.find(id) != ui_windows.end()) {
        OE_WARN("UI window [{}] already exists", title);
        return false;
      }

      ui_windows[id] = std::move(window);
      ui_windows[id]->Attach();

      return true;
    }

    static bool HasWindow(const std::string& title);

    static void RemoveWindow(const std::string& title);
    static void RemoveWindow(const UUID& id);

    static void UpdateWindows(float dt);

    static void BeginFrame();
    static void EndFrame();

    static void Shutdown();

    static ImGuiContext* GetContext();

   private:
    static bool enabled;
    static std::string ini_path;

    static UITheme active_theme;

    static Scope<WindowRef> windowref;
    static ImGuiContext* ui_context;

    static std::unordered_map<UUID, Scope<UIWindow>> ui_windows;

    static void SetTheme();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_UI_HPP
