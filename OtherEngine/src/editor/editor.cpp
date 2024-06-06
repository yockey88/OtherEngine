/**
 * \file editor/editor.cpp
*/
#include "editor/editor.hpp"

#include <filesystem>
#include <glad/glad.h>
#include <imgui/imgui.h>

#include "core/engine.hpp"
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "event/event_queue.hpp"
#include "event/core_events.hpp"
#include "project/project.hpp"
#include "parsing/ini_parser.hpp"
#include "rendering/renderer.hpp"
#include "layers/debug_layer.hpp"
#include "layers/editor_core.hpp"
#include "editor/project_panel.hpp"
#include "editor/selection_manager.hpp"

namespace other {
  
  Editor::Editor(Engine* engine , Scope<App>& app)
      : App(engine) , cmdline(engine->cmd_line) , engine_config(engine->config) , 
        app(std::move(app)) {
    project = Project::Create(engine->cmd_line , engine->config);     
  } 
      
  void Editor::OnAttach() {
    if (app == nullptr) {
      OE_ERROR("Attempting to attach a null application to the editor");
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE);
      return;
    }

    try {
      /// needs to be a better way to find the editor config file
      Path p = Filesystem::FindCoreFile(std::filesystem::path("editor.other"));
      IniFileParser parser(p.string());
      editor_config = parser.Parse();
    } catch (const IniException& e) {
      OE_ERROR("Failed to parse editor configuration file : {}", e.what());
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE);
      return;
    }

    Renderer::GetWindow()->ForceResize({ 1920 , 1080 });

    viewport = NewScope<Framebuffer>();

    /// TODO: 
    ///  - move to panel manager so that we can generalize panel creation
    ///     and allow for user created panels
    project_panel = Ref<ProjectPanel>::Create(*this);
    scene_panel = Ref<ScenePanel>::Create(*this);
    entity_properties_panel = Ref<EntityProperties>(*this);

    /// tbh should get rid of OnProjectChange, engine should be focused on one project at a time
    ///   and to reload we should either recompile or simply shut down and reload a new file
    ///   (preferably reload new file so we can keep everything at runtime)
    project_panel->OnProjectChange(project);
    /// end panel manager section
     
    /// editor should always push debug layer to allow for debug control 
    Ref<Layer> debug_layer = NewRef<DebugLayer>(this);
    PushLayer(debug_layer);

    Ref<Layer> editor_layer = NewRef<EditorCore>(this);
    PushLayer(editor_layer);

    /// here we want to load the application but not attach it
    ///   i.e. we want the app to be prepared for the mock 'runtime' but not
    ///   fully running
    app->OnLoad();

    if (SelectionManager::HasSelection()) {
      OE_INFO("Why tf we have a selection???");
    }
  }

  void Editor::OnEvent(Event* event) {
    app->OnEvent(event);
  }

  void Editor::Update(float dt) {
    entity_properties_open = SelectionManager::HasSelection();
  }

  void Editor::Render() {
    viewport->BindFrame();
    app->Render();
    viewport->UnbindFrame();
  }

  void Editor::RenderUI() {
    app->RenderUI();

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        ImGui::EndMenu();
      }
      
      if (ImGui::BeginMenu("Edit")) {
        ImGui::EndMenu();
      }
      
      if (ImGui::BeginMenu("Assets")) {
        ImGui::EndMenu();
      }
      
      if (ImGui::BeginMenu("Objects")) {
        ImGui::EndMenu();
      }

      ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Viewport")) {
      if (viewport != nullptr && viewport->Valid()) {
        auto size = ImGui::GetContentRegionAvail();
        ImGui::Image((void*)(intptr_t)viewport->Texture() , size , ImVec2(0 , 1) , ImVec2(1 , 0)); 
      } else {
        ImGui::Text("No Viewport Generated");
      }
    }
    ImGui::End();

    project_panel->OnGuiRender(project_panel_open);
    scene_panel->OnGuiRender(scene_panel_open);

    entity_properties_panel->OnGuiRender(entity_properties_open);
    if (SelectionManager::HasSelection() && !entity_properties_open) {
      SelectionManager::ClearSelection();
    }

    if (ImGui::Begin("Inspector")) {

    }
    ImGui::End();
  }

  void Editor::OnDetach() {
    viewport = nullptr;
    app->OnUnload();
  }

  void Editor::OnSceneLoad(const SceneMetadata* metadata) {
    OE_DEBUG("Editor::OnSceneLoad({})" , metadata->path);
    project_panel->SetSceneContext(metadata->scene);
    scene_panel->SetSceneContext(metadata->scene);  
    entity_properties_panel->SetSceneContext(metadata->scene);
  }

} // namespace other
