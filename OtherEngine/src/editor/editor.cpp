/**
 * \file editor/editor.cpp
*/
#include "editor/editor.hpp"

#include <filesystem>
#include <fstream>

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
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "layers/debug_layer.hpp"
#include "layers/editor_core.hpp"
#include "scene/scene_serializer.hpp"
#include "scripting/script_engine.hpp"
#include "scripting/lua/lua_module.hpp"
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

    /// TODO: 
    ///  - move to panel manager so that we can generalize panel creation
    ///     and allow for user created panels
    project_panel = Ref<ProjectPanel>::Create(*this);
    scene_panel = Ref<ScenePanel>::Create(*this);
    entity_properties_panel = Ref<EntityProperties>::Create(*this);

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
    is_editor = true;
    app->SetInEditor();
    app->OnLoad();

    /// load editor scripts
    ///  get the module
    ///  ... and then idrk ????
    lua_module = ScriptEngine::GetModuleAs<LuaModule>(LanguageModuleType::LUA_MODULE);

    editor_camera = Ref<PerspectiveCamera>::Create();
    editor_camera->SetPosition({ 0.f , 0.f , 3.f });
    editor_camera->SetDirection({ 0.f , 0.f , -1.f });
    editor_camera->SetUp({ 0.f , 1.f , 0.f });
    Renderer::BindCamera(editor_camera);
  }

  void Editor::OnEvent(Event* event) {
    app->OnEvent(event);
  }

  void Editor::Update(float dt) {
    entity_properties_open = SelectionManager::HasSelection();

    if (playing) {
      app->Update(dt);
    }
  }

  void Editor::Render() {
    app->Render();
  }

  void Editor::RenderUI() {
    app->RenderUI();

    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Reload")) {
          ScriptEngine::ReloadAllScripts();
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

    project_panel->OnGuiRender(project_panel_open);
    scene_panel->OnGuiRender(scene_panel_open);

    entity_properties_panel->OnGuiRender(entity_properties_open);
    if (SelectionManager::HasSelection() && !entity_properties_open) {
      SelectionManager::ClearSelection();
    }

    if (ImGui::Begin("Inspector") && scene_manager->HasActiveScene()) {
      if (!playing && ImGui::Button("Play")) {
        scene_manager->StartScene();
        playing = true; 
      } else if (playing && ImGui::Button("Stop Scene")) {
        scene_manager->StopScene();
        playing = false;
      }

      if (ImGui::Button("Save Scene")) {
        Path active_path = scene_manager->ActiveScene()->path;
        std::string scene_name = scene_manager->ActiveScene()->name;
        Ref<Scene> scene = scene_manager->ActiveScene()->scene;

        UnloadScene();

        SceneSerializer serializer;
        std::stringstream ss;
        serializer.Serialize(scene_name , ss , scene);

        if (ss.str().size() == 0) {
          OE_WARN("Failed to serialize scene!");
        } else {
          std::ofstream scn_file(active_path);
          if (!scn_file.is_open()) {
            OE_ERROR("Failed to open scene file for scene {}" , scene_name);
          } else {
            scn_file << ss.str();
          }
        }

        LoadScene(active_path);
      }
    }

    ImGui::End();
  }

  void Editor::OnDetach() {
    app->OnUnload();

    project_panel = nullptr;
    scene_panel = nullptr;
    entity_properties_panel = nullptr;
  }

  void Editor::OnSceneLoad(const SceneMetadata* metadata) {
    OE_DEBUG("Editor::OnSceneLoad({})" , metadata->path);

    project_panel->SetSceneContext(metadata->scene);
    scene_panel->SetSceneContext(metadata->scene);  
    entity_properties_panel->SetSceneContext(metadata->scene);
  }
      
  void Editor::OnSceneUnload() {
    project_panel->SetSceneContext(nullptr);
    scene_panel->SetSceneContext(nullptr);
    entity_properties_panel->SetSceneContext(nullptr);

    playing = false;
    OE_DEBUG("Scene Context Unloaded");
  }

} // namespace other
