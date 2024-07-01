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
#include "event/app_events.hpp"
#include "event/event_handler.hpp"

#include "parsing/ini_parser.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "layers/debug_layer.hpp"
#include "layers/editor_core.hpp"

namespace other {
  
  Editor::Editor(Engine* engine , Scope<App>& app)
      : App(engine) , cmdline(engine->cmd_line) , engine_config(engine->config) , 
        app(std::move(app)) {
    is_editor = true;
    in_editor = false;
  } 
      
  void Editor::SaveActiveScene() {
    OE_ASSERT(ActiveScene() != nullptr , "Attempting save a null scene");
    Path p = ActiveScene()->path;

    scene_manager->SaveActiveScene();
    UnloadScene();
    LoadScene(p);
  }

  void Editor::OnLoad() {
    panel_manager = NewScope<PanelManager>();
    panel_manager->Load(this , GetProjectContext());
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

      OE_DEBUG("loaded editor config from {}" , p.string());
    } catch (const IniException& e) {
      OE_ERROR("Failed to parse editor configuration file : {}", e.what());
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE);
      return;
    }
    

    Renderer::GetWindow()->ForceResize({ 1920 , 1080 });

    /// editor should always push debug layer to allow for debug control 
    Ref<Layer> debug_layer = NewRef<DebugLayer>(this);
    PushLayer(debug_layer);

    Ref<Layer> editor_layer = NewRef<EditorCore>(this);
    PushLayer(editor_layer);

    /// tell the application we are holding we are in the editor
    /// NOTE: this only sets a flag, does nothing else yet, but will be nice in the future when 
    ///   running simulations and such
    app->SetInEditor();

    panel_manager->Attach(engine_config);

    editor_camera = Ref<PerspectiveCamera>::Create();
    editor_camera->SetPosition({ 0.f , 0.f , 3.f });
    editor_camera->SetDirection({ 0.f , 0.f , -1.f });
    editor_camera->SetUp({ 0.f , 1.f , 0.f });
    Renderer::BindCamera(editor_camera);

    OE_DEBUG("Editor attached");
  }

  void Editor::OnEvent(Event* event) {
    EventHandler handler(event);
    handler.Handle<ProjectDirectoryUpdateEvent>([this](ProjectDirectoryUpdateEvent& e) -> bool {
      project_metadata->CreateScriptWatchers();

      if (!project_metadata->RegenProjectFile()) {
        OE_ERROR("Failed to generate project files! Project metadata corrupted!");
        return false;
      } 

      ReloadScripts();

      return true;
    });

    handler.Handle<ScriptReloadEvent>([this](ScriptReloadEvent& e) -> bool {
      ReloadScripts();
      return true;
    });

    panel_manager->OnEvent(event);
  }
      
  void Editor::EarlyUpdate(float dt) {
    /// go through and trigger any events to dispatch in the next call
    if (Renderer::IsWindowFocused() && lost_window_focus) {
      lost_window_focus = false;

      bool project_directories_changed = false;
      if (project_metadata->EditorDirectoryChanged()) {
        EventQueue::PushEvent<ProjectDirectoryUpdateEvent>(EDITOR_DIR);
        project_directories_changed = true;
      }

      if (project_metadata->ScriptDirectoryChanged()) {
        EventQueue::PushEvent<ProjectDirectoryUpdateEvent>(SCRIPT_DIR);
        project_directories_changed = true;
      }

      if (!project_directories_changed && project_metadata->AnyScriptChanged()) {
        EventQueue::PushEvent<ScriptReloadEvent>();
      }
    } else {
      lost_window_focus = true;
    }
  }

  void Editor::Update(float dt) {
    panel_manager->Update(dt);
  }

  void Editor::Render() {
    panel_manager->Render();
  }

  void Editor::RenderUI() {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Reload")) {
          ReloadScripts();
        }

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
      if (Renderer::Viewport() != nullptr && Renderer::Viewport()->Valid()) {
        ImVec2 curr_win_size = ImGui::GetWindowSize();
        float aspect_ratio = Renderer::GetWindow()->AspectRatio();
        ImVec2 size{};
        size.x = curr_win_size.y * aspect_ratio;
        size.y = curr_win_size.x / aspect_ratio;

        if (curr_win_size.y >= size.y) {
          size.y = curr_win_size.x;
        } else {
          size.y = curr_win_size.y;
        }

        Renderer::Viewport()->Resize({ size.x , size.y });

        ImVec2 cursor_pos = {
          (curr_win_size.x - size.x) * 0.5f ,
          ((curr_win_size.y - size.y) * 0.5f) + 7.f
        };

        ImGui::SetCursorPos(cursor_pos);


        uint32_t tex_id = Renderer::Viewport()->Texture();
        ImGui::Image((void*)(uintptr_t)tex_id , size , ImVec2(0 , 1) , ImVec2(1 , 0)); 
      } else {
        ImGui::Text("No Viewport Generated");
      }
    }
    ImGui::End();

    if (ImGui::Begin("Inspector") && scene_manager->HasActiveScene()) {
      if (!playing && ImGui::Button("Play")) {
        scene_manager->StartScene();
        playing = true; 
      } else if (playing && ImGui::Button("Stop Scene")) {
        scene_manager->StopScene();
        playing = false;

        Renderer::BindCamera(editor_camera);
      }

      if (ImGui::Button("Save Scene")) {
        SaveActiveScene();
      }
    }

    panel_manager->RenderUI();

    ImGui::End();
  }
      
  void Editor::LateUpdate(float dt) {
    if (!HasActiveScene()) {
      return;
    }
  }

  void Editor::OnDetach() {
    OE_DEBUG("Detaching editor");

    panel_manager->Detach();

    OE_DEBUG("Editor detached");
  }

  void Editor::OnUnload() {
    panel_manager->Unload();
  }

  void Editor::OnSceneLoad(const SceneMetadata* metadata) {
    OE_DEBUG("Editor::OnSceneLoad({})" , metadata->path);

    panel_manager->OnSceneLoad(metadata);
  }
      
  void Editor::OnSceneUnload() {
    panel_manager->OnSceneUnload();

    playing = false;
    OE_DEBUG("Scene Context Unloaded");
  }

  void Editor::OnScriptReload() {
    panel_manager->OnScriptReload();
  }

} // namespace other
