/**
 * \file editor_layer.cpp
 **/
#include "editor/test_editor_layer.hpp"

#include "core/config_keys.hpp"
#include "core/filesystem.hpp"
#include "input/keyboard.hpp"
#include "event/event.hpp"
#include "event/event_handler.hpp"
#include "event/app_events.hpp"
#include "event/key_events.hpp"
#include "application/app_state.hpp"

#include "rendering/camera_base.hpp"
#include "scripting/script_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/geometry_pass.hpp"
#include "rendering/perspective_camera.hpp"

namespace other {

  void TEditorLayer::OnAttach() {
    scene_renderer = CreateRenderer();
    editor_camera = NewRef<PerspectiveCamera>(Renderer::WindowSize());
    editor_camera->SetPosition({ 0.f , 0.f , 3.f });
    editor_camera->SetDirection({ 0.f , 0.f , -1.f });

    auto win_size = Renderer::WindowSize();
    editor_camera->SetViewport(win_size);
    DefaultUpdateCamera(editor_camera);

    LoadScripts();
    for (auto& [id , script] : editor_scripts.scripts) {
      script->Initialize();
      script->Start();
    }
  }

  void TEditorLayer::OnDetach() {
    for (auto& [id , script] : editor_scripts.scripts) {
      script->Stop();
      script->Shutdown();
      script->OnBehaviorUnload();
    }
    editor_scripts.scripts.clear();
  }

  void TEditorLayer::OnEarlyUpdate(float dt) {
    AppState::Scenes()->EarlyUpdateScene(dt);

    for (auto& [id , script] : editor_scripts.scripts) {
      script->EarlyUpdate(dt);
    }
  }

  void TEditorLayer::OnUpdate(float dt) {
    AppState::Scenes()->UpdateScene(dt);

    for (auto& [id , script] : editor_scripts.scripts) {
      script->Update(dt);
    }
  }

  void TEditorLayer::OnLateUpdate(float dt) {
    AppState::Scenes()->LateUpdateScene(dt);

    for (auto& [id , script] : editor_scripts.scripts) {
      script->LateUpdate(dt);
    }
  }

  void TEditorLayer::OnRender() {
    bool scene_active = AppState::Scenes()->RenderScene(scene_renderer , editor_camera);
    if (scene_active) {
      const auto& frames = scene_renderer->GetRender(); 
      const auto& vp = frames.at(FNV("Geometry"));
      other::Renderer::DrawFramebufferToWindow(vp);
    } 
  }

  void TEditorLayer::OnUIRender() {
    AppState::Scenes()->RenderSceneUI();

    for (auto& [id , script] : editor_scripts.scripts) {
      script->RenderUI();
    }
    
    // panel_manager->RenderUI();
  }

  void TEditorLayer::OnEvent(Event* event) {
    EventHandler handler(event);

    handler.Handle<ScriptReloadEvent>([this](ScriptReloadEvent& e) -> bool {
      ReloadScripts();
      return true;
    });
    
    handler.Handle<KeyPressed>([this](KeyPressed& key) -> bool {
      if (key.Key() == Keyboard::Key::OE_ESCAPE) {
        // if (playing) {
        //   AppState::Scenes()->StopScene();
        //   playing = false;
        // }

        // Mouse::FreeCursor();
        // camera_free = false;
      
        // return true;
      } 

      if (Keyboard::LCtrlLayerKey(Keyboard::Key::OE_W)) {
        scene_renderer->ToggleWireframe();
      }

      return false;
    });
  }
 
  void TEditorLayer::OnSceneLoad(const SceneMetadata* metadata) {}

  void TEditorLayer::OnSceneUnload() {}
 
  void TEditorLayer::OnScriptReload() {
    editor_scripts.scripts.clear();
    for (const auto& [id , info] : editor_scripts.data) {
      Ref<ScriptModule> mod = ScriptEngine::GetScriptModule(info.module);
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

      // ScriptObject* inst = mod->GetScriptObject(name , nspace);
      // if (inst == nullptr) {
      //   OE_ERROR("Failed to get script {} from script module {}" , name , info.module);
      //   continue;
      // } else {
      //   std::string case_ins_name;
      //   std::transform(name.begin() , name.end() , std::back_inserter(case_ins_name) , ::toupper);

      //   UUID id = FNV(case_ins_name);
      //   auto& obj = editor_scripts.scripts[id] = inst;
      //   obj->Start();
      // }
    }
  }

  void TEditorLayer::LoadScripts() {
    auto proj_context = AppState::ProjectContext();
    editor_scripts = ScriptEngine::LoadScriptsFromTable(proj_context->config , kEditorSection);
  }

  Ref<SceneRenderer> TEditorLayer::CreateRenderer() {
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

    PipelineSpec base_spec = {
      .framebuffer_spec = {
        .depth_func = other::LESS_EQUAL ,
        .clear_color = { 0.1f , 0.1f , 0.1f , 1.f } ,
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
      .model_uniforms = model_unis , 
      .model_binding_point = model_binding_pnt ,
      .material_uniforms = material_unis ,
      .material_binding_point = material_binding_pnt ,
    };

    PipelineSpec geometry_spec = base_spec;
    geometry_spec.debug_name = "Geometry";
    
    PipelineSpec debug_spec = base_spec;
    debug_spec.debug_name = "Debug";

    SceneRenderSpec spec{
      .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
      .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt) ,
      .passes  {
        geom_pass
      } ,
      .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV(geom_pass->Name()) } } ,
          { FNV("Debug")    , { FNV(geom_pass->Name()) } } ,
      } ,
    }; 

    spec.pipelines.push_back(geometry_spec);
    spec.pipelines.push_back(debug_spec);

    return NewRef<SceneRenderer>(spec);
  }

} // namespace other
