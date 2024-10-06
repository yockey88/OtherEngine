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

#include "scripting/script_engine.hpp"
#include "rendering/vertex.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/uniform.hpp"
#include "rendering/renderer.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/geometry_pass.hpp"
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
    DefaultUpdateCamera(editor_camera);
    
    READ_INI_INTO(editor , editor_config , "editor.other");
    
    Renderer::GetWindow()->ForceResize({ 1920 , 1080 });
    editor_camera->SetViewport({ 1920 , 1080 });

    LoadEditorScripts(editor_config);

    editor_scripts.ApiCall("NativeInitialize");
    editor_scripts.ApiCall("OnInitialize");

    editor_scripts.ApiCall("NativeStart");
    editor_scripts.ApiCall("OnStart");

    panel_manager = NewScope<PanelManager>();
    panel_manager->Attach((Editor*)ParentApp() , AppState::ProjectContext() , editor_config);

    default_renderer = GetDefaultRenderer();
  }

  void EditorLayer::OnDetach() {
    editor_scripts.ApiCall("OnStop");
    editor_scripts.ApiCall("NativeStop");
    editor_scripts.ApiCall("OnShutdown");
    editor_scripts.ApiCall("NativeShutdown");
    editor_scripts.ApiCall("OnBehaviorUnload");
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

    editor_scripts.ApiCall("EarlyUpdate" , dt);

    AppState::Scenes()->EarlyUpdateScene(dt);
  }

  void EditorLayer::OnUpdate(float dt) {
    panel_manager->Update(dt);

    /// after all early updates, update client and script
    editor_scripts.ApiCall("Update" , dt);

    AppState::Scenes()->UpdateScene(dt);
  }
  
  void EditorLayer::OnLateUpdate(float dt) {
    panel_manager->LateUpdate(dt);

    /// after all early updates, update client and script
    editor_scripts.ApiCall("LateUpdate" , dt);

    AppState::Scenes()->LateUpdateScene(dt);
    if (camera_free && !playing) {
      DefaultUpdateCamera(editor_camera);
    }
  }

  void EditorLayer::OnRender() {
    /// TODO add the ability to see scene without editor camera
    bool scene_active = AppState::Scenes()->RenderScene(default_renderer , editor_camera);
    if (scene_active) {
      return;
    }

    /// render default something or other
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
    if (HasActiveScene() && ImGui::Begin("Viewport")) {
      auto& frames = default_renderer->GetRender();
      viewport = nullptr;

      if (auto itr = frames.find(FNV("Geometry")); itr != frames.end()) {
        viewport = itr->second;
      }

      if (viewport == nullptr) {
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
    
      ImGui::End();
    }

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
    }
    ImGui::End();

    editor_scripts.ApiCall("RenderUI");

    panel_manager->RenderUI();
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
      if (key.Key() == Keyboard::Key::OE_ESCAPE) {
        if (playing) {
          AppState::Scenes()->StopScene();
          playing = false;
        }

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
      if (e.Button() == Mouse::MIDDLE && !playing) {
        Mouse::LockCursor();
        camera_free = true;
        DefaultUpdateCamera(editor_camera);
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
    // editor_scripts.Clear();
    // for (const auto& [id , info] : editor_scripts.data) {
    //   Ref<ScriptModule> mod = ScriptEngine::GetScriptModule(info.module);
    //   if (mod == nullptr) {
    //     OE_ERROR("Failed to find editor scripting module {} [{}]" , info.module , FNV(info.module));
    //     continue;
    //   }

    //   std::string nspace = "";
    //   std::string name = info.obj_name;
    //   if (name.find("::") != std::string::npos) {
    //     nspace = name.substr(0 , name.find_first_of(":"));
    //     OE_DEBUG("Editor script from namespace {}" , nspace);

    //     name = name.substr(name.find_last_of(":") + 1 , name.length() - nspace.length() - 2);
    //     OE_DEBUG(" > with name {}" , name);
    //   }

    //   // ScriptObject* inst = mod->GetScriptObject(name , nspace);
    //   // if (inst == nullptr) {
    //   //   OE_ERROR("Failed to get script {} from script module {}" , name , info.module);
    //   //   continue;
    //   // } else {
    //   //   std::string case_ins_name;
    //   //   std::transform(name.begin() , name.end() , std::back_inserter(case_ins_name) , ::toupper);

    //   //   UUID id = FNV(case_ins_name);
    //   //   auto& obj = editor_scripts.scripts[id] = inst;
    //   //   obj->Start();
    //   // }
    // }

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
    OE_DEBUG("Retrieving editor scripts");
    editor_scripts = ScriptEngine::LoadScriptsFromTable(editor_config , kEditorSection);
  }
      
  Ref<SceneRenderer> EditorLayer::GetDefaultRenderer() {
    uint32_t camera_binding_pnt = 0;
    std::vector<Uniform> cam_unis = {
      { "projection" , other::ValueType::MAT4 } ,
      { "view"       , other::ValueType::MAT4 } ,
      { "viewpoint"  , other::ValueType::VEC4 } ,
    };

    uint32_t model_binding_pnt = 1;
    std::vector<Uniform> model_unis = {
      { "models" , other::ValueType::MAT4 , 100 } ,
    };
    
    uint32_t material_binding_pnt = 2;
    std::vector<Uniform> material_unis = {
      { "materials" , other::ValueType::USER_TYPE , 100 , sizeof(other::Material) } ,
    };
    
    uint32_t light_binding_pnt = 3;
    std::vector<Uniform> light_unis = {
      { "num_lights" , other::ValueType::VEC4 } ,
      { "direction_lights" , other::ValueType::USER_TYPE , 100 , sizeof(other::DirectionLight) } ,
      { "point_lights" , other::ValueType::USER_TYPE , 100 , sizeof(other::PointLight) } ,
    };

    glm::vec2 window_size = Renderer::WindowSize();
      
    const Path shader_dir = Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const Path geom_shader_path = shader_dir / "default.oshader"; 
    std::vector<Uniform> geometry_unis = {
    };
    Ref<Shader> geometry_shader = BuildShader(geom_shader_path);
    Ref<RenderPass> geom_pass = NewRef<GeometryPass>(geometry_unis , geometry_shader);

    Layout layout{
      { other::ValueType::VEC3 , "position" } ,
      { other::ValueType::VEC3 , "normal"   } ,
      { other::ValueType::VEC3 , "tangent"  } ,
      { other::ValueType::VEC3 , "binormal" } ,
      { other::ValueType::VEC2 , "uvs"      } ,
    };

    FramebufferSpec fb_spec{
      .depth_func = other::LESS_EQUAL ,
      .clear_color = { 0.1f , 0.1f , 0.1f , 1.f } ,
      .size =  {
        window_size.x ,
        window_size.y ,
      },
    };

    SceneRenderSpec spec{
      .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
      .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt) ,
      .pipelines = {
        {
          .framebuffer_spec = fb_spec ,
          .vertex_layout = layout,
          .model_uniforms = model_unis , 
          .model_binding_point = model_binding_pnt ,
          .material_uniforms = material_unis ,
          .material_binding_point = material_binding_pnt ,
          .debug_name = "Geometry" , 
        } ,
        {
          .framebuffer_spec = fb_spec ,
          .vertex_layout = layout,
          .model_uniforms = model_unis , 
          .model_binding_point = model_binding_pnt ,
          .material_uniforms = material_unis ,
          .material_binding_point = material_binding_pnt ,
          .debug_name = "Debug" ,
        }
      } ,
      .passes  {
        geom_pass
      } ,
      .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV(geom_pass->Name()) } } ,
      } ,
    }; 
    return NewRef<SceneRenderer>(spec);
  }
      
  void EditorLayer::LaunchSettingsWindow() {
    Ref<UIWindow> settings_window = NewRef<SettingsWindow>();
    ParentApp()->PushUIWindow(settings_window);
  }

} // namespace other
