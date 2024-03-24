/**
 * \file launcher_layer.cpp
 **/
#include "launcher_layer.hpp"

#include "rendering/ui/file_explorer.hpp"

namespace other {

  constexpr static std::string_view kMainMenuBarName = "CoreLayerMainMenuBar";
  constexpr static uint64_t kMainMenuBarID = FNV("CoreLayerMainMenuBar"); 

  constexpr static std::string_view kFileExplorerName = "CoreLayerFileExplorer";
  constexpr static uint64_t kFileExplorerID = FNV("CoreLayerFileExplorer");

  void LauncherLayer::OnAttach() {
  }

  void LauncherLayer::OnUpdate(float dt) {
    if (selection_context.selected_path.has_value()) {
      auto path = selection_context.selected_path.value();

    }
  }

  void LauncherLayer::OnRender() {
  }

  void LauncherLayer::OnUIRender() {
    auto fe_option = [this](const std::string& option , FileExplorerType type) {
      if (ImGui::MenuItem(option.c_str())) { 
        FileExplorer file_explorer(kFileExplorerName.data() , "." , type);
        file_explorer.OnAttach();

        selection_context.selected_path = file_explorer.GetPath();
      }
    };

    bool main_bar_open = ImGui::BeginMainMenuBar();
    if (!main_bar_open) {
      return;
    }

    bool file_menu_open = ImGui::BeginMenu("File");
    if (!file_menu_open) {
      ImGui::EndMainMenuBar();
      return;
    }

    fe_option("Open File" , FileExplorerType::OPEN_FILE);
    fe_option("Open Folder" , FileExplorerType::OPEN_FOLDER);
    fe_option("Save File" , FileExplorerType::SAVE_FILE);

    ImGui::EndMenu();
    ImGui::EndMainMenuBar();

    for (auto& [id , window] : windows) {
      if (window->IsOpen()) {
        window->Render();
      }
    }
  }

} // namespace other
