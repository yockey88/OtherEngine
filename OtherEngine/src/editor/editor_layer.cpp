/**
 * \file editor/editor_layer.cpp
 **/
#include "editor/editor_layer.hpp"

#include "core/config_keys.hpp"
#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "core/config.hpp"
#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"
#include "event/mouse_events.hpp"
#include "parsing/ini_parser.hpp"
#include "input/keyboard.hpp"

#include "event/app_events.hpp"
#include "event/key_events.hpp"
#include "event/event_handler.hpp"
#include "event/event_queue.hpp"

#include "rendering/camera_base.hpp"
#include "scripting/script_engine.hpp"

#include "rendering/renderer.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "editor/editor.hpp"
#include "editor/editor_settings.hpp"
#include "editor/editor_images.hpp"

namespace other {

std::vector<float> fb_verts = {
   1.f ,  1.f , 1.f , 1.f ,
  -1.f ,  1.f , 0.f , 1.f ,
  -1.f , -1.f , 0.f , 0.f ,
   1.f , -1.f , 1.f , 0.f
};

std::vector<uint32_t> fb_indices{ 0 , 1 , 3 , 1 , 2 , 3 };
std::vector<uint32_t> fb_layout{ 2 , 2 };

  void EditorLayer::OnAttach() {
    /// load editor icons and data
    EditorImages::Initialize();

    editor_camera = NewRef<PerspectiveCamera>(Renderer::WindowSize());
    editor_camera->SetPosition({ 0.f , 0.f , 3.f });
    editor_camera->SetDirection({ 0.f , 0.f , -1.f });
    
    READ_INI_INTO(editor , editor_config , "editor.other");
    
    Renderer::GetWindow()->ForceResize({ 1920 , 1080 });
    editor_camera->SetViewport({ 1920 , 1080 });

    LoadEditorScripts(editor_config);

    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Initialize();
      script->Start();
    }

    panel_manager = NewScope<PanelManager>();
    panel_manager->Attach((Editor*)ParentApp() , AppState::ProjectContext() , editor_config);
      
    model_handle = ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
    model = AssetManager::GetAsset<StaticModel>(model_handle);
    model_source = model->GetModelSource();

    default_renderer = GetDefaultRenderer();

    const Path fbshader_path = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "fbshader.oshader";
    fbshader = BuildShader(fbshader_path);
    fb_mesh = NewScope<VertexArray>(fb_verts , fb_indices , fb_layout);
  }

  void EditorLayer::OnDetach() {
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Stop();
      script->Shutdown();
      script->OnBehaviorUnload();
    }
    editor_scripts.scripts.clear();
    panel_manager->Detach();

    EditorImages::Shutdown();
  }
      
  void EditorLayer::OnEarlyUpdate(float dt) {
    /// go through and trigger any events to dispatch in the next call
    if (Renderer::IsWindowFocused() && lost_window_focus) {
      lost_window_focus = false;

      bool project_directories_changed = false;
      if (AppState::ProjectContext()->EditorDirectoryChanged()) {
        EventQueue::PushEvent<ProjectDirectoryUpdateEvent>(EDITOR_DIR);
        project_directories_changed = true;
      }

      if (AppState::ProjectContext()->ScriptDirectoryChanged()) {
        EventQueue::PushEvent<ProjectDirectoryUpdateEvent>(SCRIPT_DIR);
        project_directories_changed = true;
      }

      if (!project_directories_changed && AppState::ProjectContext()->AnyScriptChanged()) {
        EventQueue::PushEvent<ScriptReloadEvent>();
      }
    } else {
      lost_window_focus = true;
    }

    panel_manager->EarlyUpdate(dt);

    AppState::Scenes()->EarlyUpdateScene(dt);
  }

  void EditorLayer::OnUpdate(float dt) {
    if (camera_free) {
      DefaultUpdateCamera(editor_camera);
    }
    
    panel_manager->Update(dt);

    /// after all early updates, update client and script
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->Update(dt);
    }

    AppState::Scenes()->UpdateScene(dt);
  }
  
  void EditorLayer::OnLateUpdate(float dt) {
    if (camera_free) {
      // DefaultLateUpdateCamera(editor_camera);
    }
    
    panel_manager->LateUpdate(dt);



    /// after all early updates, update client and script
    for (const auto& [id , script] : editor_scripts.scripts) {
      script->LateUpdate(dt);
    }

    AppState::Scenes()->LateUpdateScene(dt);
  }

  void EditorLayer::OnRender() {
    // for (const auto& [id , script] : editor_scripts.scripts) {
    //   script->Render();
    // }
    // panel_manager->Render();

    if (HasActiveScene()) {
      AppState::Scenes()->RenderScene(default_renderer , editor_camera);
    } else {
      
    }
    default_renderer->EndScene();

    // default_renderer->BeginScene(editor_camera);
    // default_renderer->SubmitStaticModel(model , model1);
    // default_renderer->EndScene();

#if 0
    auto& frames = default_renderer->GetRender();
    viewport = frames.at(FNV("GeometryPipeline"));

    fbshader->Bind();
    fbshader->SetUniform("screen_tex" , 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D , viewport->texture);

    fb_mesh->Draw(TRIANGLES);
#endif
  }

  void EditorLayer::OnUIRender() {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Reload")) {
          ReloadScripts();
        }

        if (ImGui::MenuItem("Settings")) {
          LaunchSettingsWindow();
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

#if 1
    if (ImGui::Begin("Viewport")) {
      auto& frames = default_renderer->GetRender();
      viewport = frames.at(FNV("GeometryPipeline"));

      if (HasActiveScene() && viewport == nullptr) {
        ScopedColor red_text(ImGuiCol_Text , ui::theme::red);
        ImGui::Text("No Viewport Generated");
      } else {
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

        ImVec2 cursor_pos = {
          (curr_win_size.x - size.x) * 0.5f ,
          ((curr_win_size.y - size.y) * 0.5f) + 7.f
        };

        ImGui::SetCursorPos(cursor_pos);

        editor_camera->SetViewport({ size.x , size.y });

        uint32_t tex_id = viewport->texture; 
        ImGui::Image((void*)(uintptr_t)tex_id , size , ImVec2(0 , 1) , ImVec2(1 , 0)); 
      } 
    }
    ImGui::End();

    if (ImGui::Begin("Inspector") /* && scene_manager->HasActiveScene() */) {
      if (!playing && ImGui::Button("Play")) {
        AppState::Scenes()->StartScene();
        playing = true; 
      } else if (playing && ImGui::Button("Stop Scene")) {
        AppState::Scenes()->StopScene();
        playing = false;
      }

      ImGui::Text("Camera Position :: [%f , %f , %f]" , 
                  editor_camera->Position().x , editor_camera->Position().y , editor_camera->Position().z);
      
      ImGui::Text("Camera Direction :: [%f , %f , %f]" , 
                  editor_camera->Direction().x , editor_camera->Direction().y , editor_camera->Direction().z);

      if (saved_scene.has_value() && ImGui::Button("Undo")) {
        AppState::Scenes()->LoadCapture(saved_scene.value());
        saved_scene = std::nullopt;
      } else if (ImGui::Button("Save Scene")) {
        AppState::Scenes()->SaveActiveScene();
      }
    }
    ImGui::End();

    for (const auto& [id , script] : editor_scripts.scripts) {
      script->RenderUI();
    }

    /// true => something changed
    if (panel_manager->RenderUI() && !saved_scene.has_value()) {
      saved_scene = AppState::Scenes()->CaptureScene();
    }
#endif
  }

  void EditorLayer::OnEvent(Event* event) {
    EventHandler handler(event);
    handler.Handle<ProjectDirectoryUpdateEvent>([this](ProjectDirectoryUpdateEvent& e) -> bool {
        AppState::ProjectContext()->CreateScriptWatchers();

      if (!AppState::ProjectContext()->RegenProjectFile()) {
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

    handler.Handle<KeyPressed>([this](KeyPressed& key) -> bool {
      if (key.Key() == Keyboard::Key::OE_ESCAPE && camera_free) {
        Mouse::FreeCursor();
        camera_free = false;
        return true;
      } 

      if (Keyboard::LCtrlLayer() && key.Key() == Keyboard::Key::OE_W) {
        default_renderer->ToggleWireframe();
      }

      return false;
    });

    handler.Handle<MouseButtonPressed>([this](MouseButtonPressed& e) -> bool {
      if (e.Button() == Mouse::MIDDLE && !camera_free) {
        Mouse::LockCursor();
        DefaultUpdateCamera(editor_camera);
        camera_free = true;
        return true;
      }

      return false;
    });
    
    panel_manager->OnEvent(event);
  }

  void EditorLayer::OnSceneLoad(const SceneMetadata* metadata) {
    panel_manager->OnSceneLoad(metadata);
  }

  void EditorLayer::OnSceneUnload() {
    panel_manager->OnSceneUnload();
  }

  void EditorLayer::OnScriptReload() {
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

    panel_manager->OnScriptReload();
  }

  void EditorLayer::SaveActiveScene() {
    OE_ASSERT(AppState::Scenes()->ActiveScene() != nullptr , "Attempting to save null scene!");

    auto& scenes = AppState::Scenes();
    bool is_playing = scenes->IsPlaying();

    if (is_playing) {
      scenes->StopScene();
    }

    Path p = scenes->ActiveScene()->path;

    scenes->SaveActiveScene();

    ParentApp()->UnloadScene();
    ParentApp()->LoadScene(p);

    if (is_playing) {
      scenes->StartScene();
    }
  }
  
  void EditorLayer::LoadEditorScripts(const ConfigTable& editor_config) {
    OE_DEBUG("Retrieving scripting modules");

    Ref<LanguageModule> cs_lang_mod = ScriptEngine::GetModule(LanguageModuleType::CS_MODULE);
    if (cs_lang_mod == nullptr) {
      OE_ERROR("Failed to load editor scripts, C# module is null!");
      return;
    }

    Ref<LanguageModule> lua_module = ScriptEngine::GetModule(LanguageModuleType::LUA_MODULE);
    if (lua_module == nullptr) {
      OE_ERROR("Failed to load editor scripts Lua Module is null!");
      return;
    }

    auto script_paths = editor_config.Get(kEditorSection , kScriptsValue);

    std::string script_key = std::string{ kEditorSection } + "." + std::string{ kScriptValue };
    auto script_objs = editor_config.GetKeys(script_key);

    auto project_path = AppState::ProjectContext()->GetMetadata().file_path.parent_path();
    auto assets_dir = AppState::ProjectContext()->GetMetadata().assets_dir;

    OE_DEBUG("Loading Editor Scripts");
    for (auto& mod : script_paths) {
      Path module_path = Path{ mod }; 

      std::string fname = module_path.filename().string();
      std::string mname = fname.substr(0 , fname.find_last_of('.'));
      OE_DEBUG("Loading Editor Script Module : {} ({})" , mname , module_path.string());

      if (module_path.extension() == ".dll") {
        module_path = project_path / "bin" / "Debug" / mod;
        cs_lang_mod->LoadScriptModule({
          .name = mname ,
          .paths = { module_path.string() } ,
          .type = ScriptModuleType::EDITOR_SCRIPT ,
        });
      } else if (module_path.extension() == ".lua") {
        module_path = assets_dir / "editor" / mod;
        lua_module->LoadScriptModule({
          .name = mname ,
          .paths = { module_path.string() } ,
          .type = ScriptModuleType::EDITOR_SCRIPT ,
        });
      }
    }

    for (const auto& obj : script_objs) {
      auto scripts = editor_config.Get(script_key , obj);

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
      
  Ref<SceneRenderer> EditorLayer::GetDefaultRenderer() {
    const Path shader_dir = Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const Path default_path = shader_dir / "default.oshader";

    uint32_t camera_binding_pnt = 0;
    std::vector<Uniform> cam_unis = {
      { "projection" , other::ValueType::MAT4 } ,
      { "view"       , other::ValueType::MAT4 } ,
      { "viewpoint"  , other::ValueType::VEC3 } ,
    };
    
    uint32_t model_binding_pnt = 1;
    std::vector<Uniform> model_unis = {
      { "models" , other::ValueType::MAT4 , 100 } ,
    };

    glm::vec2 window_size = Renderer::WindowSize();

    //// how tf do render passes work ??? 
    SceneRenderSpec spec{
      .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
      .passes = {
        {
          .name = "GeometryPass1" , 
          .tag_col = { 1.f , 0.f , 0.f , 1.f } ,
          .uniforms = {} ,
          .shader = other::BuildShader(default_path) ,
        } ,
      } ,
      .pipelines = {
        {
          .framebuffer_spec = {
            .depth_func = other::LESS_EQUAL ,
            .clear_color = { 0.1f , 0.3f , 0.3f , 1.f } ,
            .size =  {
              window_size.x ,
              window_size.y ,
            },
          } ,
          .vertex_layout = {
            { other::ValueType::VEC3 , "position" } ,
            { other::ValueType::VEC3 , "normal"   } ,
            { other::ValueType::VEC3 , "tangent"  } ,
            { other::ValueType::VEC3 , "binormal" } ,
            { other::ValueType::VEC2 , "uvs"      }
          } ,
          .debug_name = "GeometryPipeline" , 
        } ,
      } ,
      .pipeline_to_pass_map = {
        { FNV("GeometryPipeline") , { FNV("GeometryPass1") } } ,
      } ,
    }; 
    return NewRef<SceneRenderer>(spec);
  }
      
  void EditorLayer::LaunchSettingsWindow() {
    Ref<UIWindow> settings_window = NewRef<SettingsWindow>();
    ParentApp()->PushUIWindow(settings_window);
  }

} // namespace other
