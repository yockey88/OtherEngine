/**
 * \file launcher_layer.cpp
 **/
#include "launcher_layer.hpp"

#include <filesystem>

#include "core/defines.hpp"
#include "event/event_queue.hpp"
#include "event/core_events.hpp"
#include "project/project.hpp"
#include "rendering/ui/file_explorer.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  constexpr static std::string_view kMainMenuBarName = "CoreLayerMainMenuBar";
  constexpr static uint64_t kMainMenuBarID = FNV("CoreLayerMainMenuBar"); 

  constexpr static std::string_view kFileExplorerName = "CoreLayerFileExplorer";
  constexpr static uint64_t kFileExplorerID = FNV("CoreLayerFileExplorer");

  void LauncherLayer::OnAttach() {
    auto* cs_module = ScriptEngine::GetModule("CsLanguageModule");
    auto* script_core = cs_module->GetScriptModule(FNV("C#-Core"));
    script = script_core->GetScript("Entity" , "Other");
    if (script == nullptr) {
      OE_ERROR("LauncherLayer::OnAttach: Failed to get script from module");
    } else {
      script->Initialize();
    }

    auto* lua_module = ScriptEngine::GetModule("LuaLanguageModule");
    auto* lua_core = lua_module->GetScriptModule(FNV("Lua-Core"));
    lua_script = lua_core->GetScript("entity");
    if (lua_script == nullptr) {
      OE_ERROR("LauncherLayer::OnAttach: Failed to get script from module");
    } else {
      lua_script->Initialize();
    }
  }

  void LauncherLayer::OnUpdate(float dt) {
    if (script != nullptr) {
      script->Update(dt);
    }

    if (lua_script != nullptr) {
      lua_script->Update(dt);
    }

    if (selection_context.selected_path.has_value()) {
      auto path = selection_context.selected_path.value();

      if (path.empty()) {
        selection_context.selected_path = std::nullopt;
        return;
      }

      switch (selection_context.context_type) {
        case ContextType::CREATE_PROJECT: 
          // write project metadata to file
          // queue project
          // queue shutdown event with load new project
          // shutdown
          break;
        case ContextType::OPEN_PROJECT: {
          // queue project
          // queue shutdown event with load new project
          // shutdown
          auto path = selection_context.selected_path.value();
          // verify project exists

          Project::QueueNewProject(path);
          EventQueue::PushEvent<ShutdownEvent>(ExitCode::LOAD_NEW_PROJECT);
        } break;
        default: 
          break;
      }
    }
  }

  void LauncherLayer::OnRender() {
  }

  void LauncherLayer::OnUIRender() {
    auto fe_option = [this](const std::string& option , FileExplorerType type , ContextType context_type) {
      if (ImGui::MenuItem(option.c_str())) { 
        FileExplorer file_explorer(kFileExplorerName.data() , "." , type);
        file_explorer.OnAttach();

        selection_context.selected_path = file_explorer.GetPath();
        selection_context.context_type = context_type;
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

    fe_option("Open Project" , FileExplorerType::OPEN_FOLDER , ContextType::OPEN_PROJECT);
    fe_option("Create Project" , FileExplorerType::SAVE_FILE , ContextType::CREATE_PROJECT);

    ImGui::EndMenu();
    ImGui::EndMainMenuBar();

    for (auto& [id , window] : windows) {
      if (window->IsOpen()) {
        window->Render();
      }
    }
  }

  void LauncherLayer::OnDetach() {
    if (script != nullptr) {
      script->Shutdown();
    }

    if (lua_script != nullptr) {
      lua_script->Shutdown();
    }
  }

} // namespace other
