/**
 * \file editor/editor.cpp
*/
#include "editor/editor.hpp"

#include <filesystem>

#include <glad/glad.h>
#include <imgui/imgui.h>

#include "core/config_keys.hpp"
#include "core/engine.hpp"
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "event/event_queue.hpp"
#include "event/core_events.hpp"
#include "parsing/ini_parser.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "layers/debug_layer.hpp"
#include "layers/editor_core.hpp"
#include "scripting/script_engine.hpp"
#include "scripting/lua/lua_module.hpp"
#include "editor/project_panel.hpp"
#include "editor/selection_manager.hpp"

namespace other {
  
  Editor::Editor(Engine* engine , Scope<App>& app)
      : App(engine) , cmdline(engine->cmd_line) , engine_config(engine->config) , 
        app(std::move(app)) {
  } 
      
  void Editor::SaveActiveScene() {
    OE_ASSERT(ActiveScene() != nullptr , "Attempting save a null scene");
    Path p = ActiveScene()->path;

    scene_manager->SaveActiveScene();
    UnloadScene();
    LoadScene(p);
  }
      
  void Editor::LoadEditorScripts() {
    Ref<LanguageModule> cs_lang_mod = ScriptEngine::GetModule(LanguageModuleType::CS_MODULE);
    if (cs_lang_mod == nullptr) {
      OE_ERROR("Failed to load editor scripts, C# module is null!");
      return;
    }

    if (lua_module == nullptr) {
      OE_ERROR("Failed to load editor scripts Lua Module is null!");
      return;
    }

    auto script_paths = engine_config.Get(kEditorSection , kScriptsValue);

    std::string script_key = std::string{ kEditorSection } + "." + std::string{ kScriptValue };
    auto script_objs = engine_config.GetKeys(script_key);

    OE_DEBUG("Loading Editor Scripts");
    for (auto& mod : script_paths) {
      Path module_path = Path{ mod }; 

      std::string fname = module_path.filename().string();
      std::string mname = fname.substr(0 , fname.find_last_of('.'));
      OE_DEBUG("Loading Editor Script Module : {} ({})" , mname , module_path.string());

      if (module_path.extension() == ".dll") {
        cs_lang_mod->LoadScriptModule({
          .name = mname ,
          .paths = { module_path.string() }
        });
      } else if (module_path.extension() == ".lua") {
        lua_module->LoadScriptModule({
          .name = mname ,
          .paths = { module_path.string() }
        });
      }
    }

    for (const auto& obj : script_objs) {
      auto scripts = engine_config.Get(script_key , obj);

      ScriptModule* mod = ScriptEngine::GetScriptModule(obj);
      if (mod == nullptr) {
        OE_ERROR("Failed to find editor scripting module {} [{}]" , obj , FNV(obj));
        continue;
      }

      for (auto& s : scripts) {
        OE_DEBUG("Attaching editor script");

        std::string nspace = "";
        std::string name = s;
        if (s.find("::") != std::string::npos) {
          nspace = s.substr(0 , s.find_first_of(":"));
          OE_DEBUG("Editor script from namespace {}" , nspace);

          name = s.substr(s.find_last_of(":") + 1 , s.length() - nspace.length() - 2);
          OE_DEBUG(" > with name {}" , name);
        }

        ScriptObject* inst = mod->GetScript(name , nspace);
        if (inst == nullptr) {
          OE_ERROR("Failed to get script {} from script module {}" , s , obj);
          continue;
        } else {
          std::string case_ins_name;
          std::transform(s.begin() , s.end() , std::back_inserter(case_ins_name) , ::toupper);

          UUID id = FNV(case_ins_name);
          editor_scripts.data[id] = ScriptObjectData{
            .module = obj ,
            .obj_name = s ,
          };
          auto& obj = editor_scripts.scripts[id] = inst;
          obj->Start();
        }
      }
    }
  }

  void Editor::OnLoad() {
    /// TODO: 
    ///  - move to panel manager so that we can generalize panel creation
    ///     and allow for user created panels

    project_panel = Ref<ProjectPanel>::Create(*this);
    scene_panel = Ref<ScenePanel>::Create(*this);
    entity_properties_panel = Ref<EntityProperties>::Create(*this);
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

    /// tbh should get rid of OnProjectChange, engine should be focused on one project at a time
    ///   and to reload we should either recompile or simply shut down and reload a new file
    ///   (preferably reload new file so we can keep everything at runtime)
    project_panel->OnProjectChange(GetProjectContext());
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

    /// load editor scripts
    ///  get the module
    ///  ... and then idrk ????
    lua_module = ScriptEngine::GetModuleAs<LuaModule>(LanguageModuleType::LUA_MODULE);

    LoadEditorScripts();
    
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->OnBehaviorLoad();
      script->Initialize();
      script->Start();
    }

    editor_camera = Ref<PerspectiveCamera>::Create();
    editor_camera->SetPosition({ 0.f , 0.f , 3.f });
    editor_camera->SetDirection({ 0.f , 0.f , -1.f });
    editor_camera->SetUp({ 0.f , 1.f , 0.f });
    Renderer::BindCamera(editor_camera);

    project_panel->OnAttach();
    scene_panel->OnAttach();
    entity_properties_panel->OnAttach();

    OE_DEBUG("Editor attached");
  }

  void Editor::OnEvent(Event* event) {
  }

  void Editor::Update(float dt) {
    entity_properties_open = SelectionManager::HasSelection();

    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Update(dt);
    }
  }

  void Editor::Render() {
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Render();
    }
  }

  void Editor::RenderUI() {
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

        Renderer::BindCamera(editor_camera);
      }

      if (ImGui::Button("Save Scene")) {
        SaveActiveScene();
      }
    }

    for (const auto& [id , script] : editor_scripts.scripts) {
      script->RenderUI();
    }

    ImGui::End();
  }
      
  void Editor::LateUpdate(float dt) {
    if (!HasActiveScene()) {
      return;
    }
  }

  void Editor::OnDetach() {
    OE_DEBUG("Detaching editor");

    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Stop();
      script->Shutdown();
      script->OnBehaviorUnload();
    }
    editor_scripts.scripts.clear();

    OE_DEBUG("Editor detached");
  }

  void Editor::OnUnload() {
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

  void Editor::OnScriptReload() {
    editor_scripts.scripts.clear();
    for (const auto& [id , info] : editor_scripts.data) {
      ScriptModule* mod = ScriptEngine::GetScriptModule(info.module);
      if (mod == nullptr) {
        OE_ERROR("Failed to find editor scripting module {} [{}]" , info.module , FNV(info.module));
        continue;
      }

      std::string nspace = "";
      std::string name = info.obj_name;
      if (name.find("::") != std::string::npos) {
        nspace = name.substr(0 , name.find_first_of(":"));
        OE_DEBUG("Editor script from namespace {}" , nspace);

        name = name.substr(name.find_last_of(":") + 1 , name.length() - nspace.length() - 2);
        OE_DEBUG(" > with name {}" , name);
      }

      ScriptObject* inst = mod->GetScript(name , nspace);
      if (inst == nullptr) {
        OE_ERROR("Failed to get script {} from script module {}" , name , info.module);
        continue;
      } else {
        std::string case_ins_name;
        std::transform(name.begin() , name.end() , std::back_inserter(case_ins_name) , ::toupper);

        UUID id = FNV(case_ins_name);
        auto& obj = editor_scripts.scripts[id] = inst;
        obj->Start();
      }
    }
  }

} // namespace other
