/**
 * \file rendering/ui/ui.cpp
 **/
#include "rendering/ui/ui.hpp"

#include <filesystem>

#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/config_keys.hpp"

namespace other {

  bool UI::enabled = false;
  std::string UI::ini_path = "";

  UITheme UI::active_theme;

  Scope<WindowRef> UI::windowref = nullptr;
  ImGuiContext* UI::ui_context = nullptr;

  std::unordered_map<UUID , Scope<UIWindow>> UI::ui_windows;

  UITheme::UITheme() {
    name = "Default";
  }

  UITheme::UITheme(const std::string& name , ImVec4 colors[UI_THEME_INDEX_COUNT]) {
    this->name = name;
    for (int i = 0; i < UI_THEME_INDEX_COUNT; i++) {
      this->colors[i] = colors[i];
    }
  }

  void UI::SetTheme() {
    // ImGuiStyle& style = ui_context->Style; 
    // style.Colors[ImGuiCol_Text] = theme.colors[UIThemeIndex::TEXT];
    // style.Colors[ImGuiCol_TextDisabled] = theme.colors[UIThemeIndex::TEXT_DISABLED];
    // style.Colors[ImGuiCol_WindowBg] = theme.colors[UIThemeIndex::WINDOW_BG];
    // style.Colors[ImGuiCol_ChildBg] = theme.colors[UIThemeIndex::CHILD_BG];
    // style.Colors[ImGuiCol_PopupBg] = theme.colors[UIThemeIndex::POPUP_BG];
    // style.Colors[ImGuiCol_Border] = theme.colors[UIThemeIndex::BORDER];
    // style.Colors[ImGuiCol_BorderShadow] = theme.colors[UIThemeIndex::BORDER_SHADOW];
    // style.Colors[ImGuiCol_FrameBg] = theme.colors[UIThemeIndex::FRAME_BG];
    // style.Colors[ImGuiCol_FrameBgHovered] = theme.colors[UIThemeIndex::FRAME_BG_HOVERED];
    // style.Colors[ImGuiCol_FrameBgActive] = theme.colors[UIThemeIndex::FRAME_BG_ACTIVE];
    // style.Colors[ImGuiCol_TitleBg] = theme.colors[UIThemeIndex::TITLE_BG];
    // style.Colors[ImGuiCol_TitleBgActive] = theme.colors[UIThemeIndex::TITLE_BG_ACTIVE];
    // style.Colors[ImGuiCol_TitleBgCollapsed] = theme.colors[UIThemeIndex::TITLE_BG_COLLAPSED];
    // style.Colors[ImGuiCol_MenuBarBg] = theme.colors[UIThemeIndex::MENU_BAR_BG];
    // style.Colors[ImGuiCol_ScrollbarBg] = theme.colors[UIThemeIndex::SCROLLBAR_BG];
    // style.Colors[ImGuiCol_ScrollbarGrab] = theme.colors[UIThemeIndex::SCROLLBAR_GRAB];
    // style.Colors[ImGuiCol_ScrollbarGrabHovered] = theme.colors[UIThemeIndex::SCROLLBAR_GRAB_HOVERED];
    // style.Colors[ImGuiCol_ScrollbarGrabActive] = theme.colors[UIThemeIndex::SCROLLBAR_GRAB_ACTIVE];
    // style.Colors[ImGuiCol_CheckMark] = theme.colors[UIThemeIndex::CHECK_MARK];
    // style.Colors[ImGuiCol_SliderGrab] = theme.colors[UIThemeIndex::SLIDER_GRAB];
    // style.Colors[ImGuiCol_SliderGrabActive] = theme.colors[UIThemeIndex::SLIDER_GRAB_ACTIVE];
    // style.Colors[ImGuiCol_Button] = theme.colors[UIThemeIndex::BUTTON];
    // style.Colors[ImGuiCol_ButtonHovered] = theme.colors[UIThemeIndex::BUTTON_HOVERED];
    // style.Colors[ImGuiCol_ButtonActive] = theme.colors[UIThemeIndex::BUTTON_ACTIVE];
    // style.Colors[ImGuiCol_Header] = theme.colors[UIThemeIndex::HEADER];
    // style.Colors[ImGuiCol_HeaderHovered] = theme.colors[UIThemeIndex::HEADER_HOVERED];
    // style.Colors[ImGuiCol_HeaderActive] = theme.colors[UIThemeIndex::HEADER_ACTIVE];
    // style.Colors[ImGuiCol_Separator] = theme.colors[UIThemeIndex::SEPARATOR];
    // style.Colors[ImGuiCol_SeparatorHovered] = theme.colors[UIThemeIndex::SEPARATOR_HOVERED];
    // style.Colors[ImGuiCol_SeparatorActive] = theme.colors[UIThemeIndex::SEPARATOR_ACTIVE];
    // style.Colors[ImGuiCol_ResizeGrip] = theme.colors[UIThemeIndex::RESIZE_GRIP];
    // style.Colors[ImGuiCol_ResizeGripHovered] = theme.colors[UIThemeIndex::RESIZE_GRIP_HOVERED];
    // style.Colors[ImGuiCol_ResizeGripActive] = theme.colors[UIThemeIndex::RESIZE_GRIP_ACTIVE];
    // style.Colors[ImGuiCol_Tab] = theme.colors[UIThemeIndex::TAB];
    // style.Colors[ImGuiCol_TabHovered] = theme.colors[UIThemeIndex::TAB_HOVERED];
    // style.Colors[ImGuiCol_TabActive] = theme.colors[UIThemeIndex::TAB_ACTIVE];
    // style.Colors[ImGuiCol_TabUnfocused] = theme.colors[UIThemeIndex::TAB_UNFOCUSED];
    // style.Colors[ImGuiCol_TabUnfocusedActive] = theme.colors[UIThemeIndex::TAB_UNFOCUSED_ACTIVE];
    // style.Colors[ImGuiCol_DockingPreview] = theme.colors[UIThemeIndex::DOCKING_PREVIEW];
    // style.Colors[ImGuiCol_DockingEmptyBg] = theme.colors[UIThemeIndex::DOCKING_EMPTY_BG];
    // style.Colors[ImGuiCol_PlotLines] = theme.colors[UIThemeIndex::PLOT_LINES];
    // style.Colors[ImGuiCol_PlotLinesHovered] = theme.colors[UIThemeIndex::PLOT_LINES_HOVERED];
    // style.Colors[ImGuiCol_PlotHistogram] = theme.colors[UIThemeIndex::PLOT_HISTOGRAM];
    // style.Colors[ImGuiCol_PlotHistogramHovered] = theme.colors[UIThemeIndex::PLOT_HISTOGRAM_HOVERED];
    // style.Colors[ImGuiCol_TableHeaderBg] = theme.colors[UIThemeIndex::TABLE_HEADER_BG];
    // style.Colors[ImGuiCol_TableBorderStrong] = theme.colors[UIThemeIndex::TABLE_BORDER_STRONG];
    // style.Colors[ImGuiCol_TableBorderLight] = theme.colors[UIThemeIndex::TABLE_BORDER_LIGHT];
    // style.Colors[ImGuiCol_TableRowBg] = theme.colors[UIThemeIndex::TABLE_ROW_BG];
    // style.Colors[ImGuiCol_TableRowBgAlt] = theme.colors[UIThemeIndex::TABLE_ROW_BG_ALT];
    // style.Colors[ImGuiCol_TextSelectedBg] = theme.colors[UIThemeIndex::TEXT_SELECTED_BG];
    // style.Colors[ImGuiCol_DragDropTarget] = theme.colors[UIThemeIndex::DRAG_DROP_TARGET];
    // style.Colors[ImGuiCol_NavHighlight] = theme.colors[UIThemeIndex::NAV_HIGHLIGHT];
    // style.Colors[ImGuiCol_NavWindowingHighlight] = theme.colors[UIThemeIndex::NAV_WINDOWING_HIGHLIGHT];
    // style.Colors[ImGuiCol_NavWindowingDimBg] = theme.colors[UIThemeIndex::NAV_WINDOWING_DIM_BG];
    // style.Colors[ImGuiCol_ModalWindowDimBg] = theme.colors[UIThemeIndex::MODAL_WINDOW_DIM_BG];
  }

  void UI::Initialize(const ConfigTable& config , const Scope<Window>& window) {
    windowref = NewScope<WindowRef>(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    auto win_ctx = windowref->main_window->Context();
    auto win_handle = win_ctx.window;
    auto ctx_handle = win_ctx.context;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(win_handle , ctx_handle);
    ImGui_ImplOpenGL3_Init("#version 460");

    ui_context = ImGui::GetCurrentContext();

    auto disabled = config.GetVal<bool>(kUiSection , kDisabledValue);
    if (!disabled.has_value() || !disabled.value()) {
      enabled = true;
    }

    auto theme = config.GetVal<std::string>(kUiSection , kThemeValue);
    if (theme.has_value()) {
      // SetTheme(theme.value());
    }

    auto ipath = config.GetVal<std::string>(kUiSection , kIniPathValue);
    if (ipath.has_value()) {
      ini_path = ipath.value();
      std::transform(ini_path.begin() , ini_path.end() , ini_path.begin() , ::tolower);
      if (std::find(ini_path.begin() , ini_path.end() , '.') == ini_path.end()) {
        ini_path += ".ini";
      }

      std::string remaining = ini_path;
      std::string dir = "";

      while (true) {
        auto fslash_pos = remaining.find_first_of('/');
        auto bslash_pos = remaining.find_first_of('\\');
        if (fslash_pos == std::string::npos && bslash_pos == std::string::npos) {
          break;
        }

        auto pos = std::min(fslash_pos , bslash_pos);
        if (!dir.empty()) {
          dir += "/";
        }
        dir += remaining.substr(0 , pos);
        remaining = remaining.substr(pos + 1);

        if (dir.empty()) {
          continue;
        }

        Path dir_path = std::filesystem::current_path() / dir;
        if (!std::filesystem::exists(dir_path)) {
          std::filesystem::create_directory(dir_path);
        }
      }

      ImGui::GetIO().IniFilename = ini_path.c_str();
    }
  }
      
  void UI::SetTheme(const UITheme& theme) {
    // this->theme = theme;
    SetTheme();
  }

  bool UI::Enabled() {
    return enabled;
  }
      
  bool UI::HasWindow(const std::string& title) {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    UUID id = FNV(title);
    return ui_windows.find(id) != ui_windows.end();
  }

  void UI::RemoveWindow(const std::string& title) {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    UUID id = FNV(title);
    if (ui_windows.find(id) == ui_windows.end()) {
      OE_WARN("UI window [{}] not found" , title);
      return;
    }

    ui_windows.erase(id);
  }

  void UI::RemoveWindow(const UUID& id) {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    if (ui_windows.find(id) == ui_windows.end()) {
      OE_WARN("UI window with id [{}] not found" , id);
      return;
    } else {
      OE_DEBUG("Removing UI window [{}]" , id);
    }

    ui_windows.erase(id);
  }
      
  void UI::UpdateWindows(float dt) {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    for (auto& [id , window] : ui_windows) {
      window->OnUpdate(dt);
    }

    auto itr = ui_windows.begin();
    for (; itr != ui_windows.end();) {
      if (!itr->second->IsOpen() && !itr->second->Pinned()) {
        OE_DEBUG("Removing UI window [{}]" , itr->second->Title());
        itr->second->OnDetach();
        itr = ui_windows.erase(itr);
        continue;
      }
      ++itr;
    }
  }

  void UI::BeginFrame() {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    const auto window_ctx = windowref->main_window->Context();
    auto win_handle = window_ctx.window;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(win_handle);
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport() , ImGuiDockNodeFlags_PassthruCentralNode);
  }

  void UI::EndFrame() {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    for (auto& [id , window] : ui_windows) {
      window->Render();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    const auto window_ctx = windowref->main_window->Context();
    auto win_handle = window_ctx.window;
    auto win_context = window_ctx.context;
    SDL_GL_MakeCurrent(win_handle , win_context);
  }

  void UI::Shutdown() {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    for (auto& [id , window] : ui_windows) {
      RemoveWindow(id);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  ImGuiContext* UI::GetContext() {
    OE_ASSERT(ui_context != nullptr , "UI context not initialized");
    OE_ASSERT(windowref != nullptr , "Window reference not initialized");
    OE_ASSERT(windowref->main_window != nullptr , "Main window not initialized");

    return ui_context;
  }

} // namespace other
