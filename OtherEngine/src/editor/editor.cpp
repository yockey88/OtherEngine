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

/// panels
#include "editor/project_panel.hpp"

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
    project_panel = Ref<ProjectPanel>::Create();
    scene_panel = Ref<ScenePanel>::Create();

    project_panel->OnProjectChange(project);
    
    Ref<Layer> debug_layer = NewRef<DebugLayer>(this);
    PushLayer(debug_layer);

    app->OnLoad();
  }

  void Editor::OnEvent(Event* event) {

    app->OnEvent(event);
  }

  void Editor::Update(float dt) {
    app->Update(dt);
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

    if (ImGui::Begin("Inspector")) {

    }
    ImGui::End();
  }

  void Editor::OnDetach() {
    viewport = nullptr;
    app->OnUnload();
  }

} // namespace other
