/**
 * \file mock_engine.cpp
 **/
#include "mock_engine.hpp"

#include <stdexcept>

#include <imgui/backends/imgui_impl_sdl2.h>

#include "core/logger.hpp"
#include "core/errors.hpp"
#include "parsing/ini_parser.hpp"

#include "input/io.hpp"

#include "application/app_state.hpp"

#include "event/event_queue.hpp"

#include "scripting/script_engine.hpp"
#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"

namespace other {

  MockEngine* MockEngine::instance = nullptr;
      
  MockEngine::MockEngine(CmdLine cmd_line , const Path& configpath) 
      : command_line(cmd_line) {
    other::IniFileParser parser(configpath.string());
    try {
      config = parser.Parse();
    } catch (other::IniException& e) {
      throw std::runtime_error(fmtstr("INI Parsing failure {}" , e.what()));
    }

    instance = this;
  } 

  MockEngine::~MockEngine() {}

  MockEngine* MockEngine::TestEngine() { 
    if (instance == nullptr) {
      throw std::logic_error("Cannot call MockEngine::TestEngine until after Gtest has begun!");
    }
    /// registered on gtest entry
    return instance;
  }

  void MockEngine::SetUp() {
    engine_stub = NewScope<Engine>(command_line);

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Main Other Engine Test Engine Thread");
    OE_TRACE("Logger initialized for unit test");

    mock_application = NewApp(engine_stub.get());
    mock_application->Load();
    AppState::Initialize(mock_application.get() , mock_application->layer_stack , 
                         mock_application->scene_manager , mock_application->asset_handler , 
                         mock_application->project_metadata);

    InitializeEngineSubSystems();
    OE_TRACE("Engine sub-systems initialized");
  }

  void MockEngine::TearDown() {
    OE_TRACE("Engine sub-systems shutdown");
    ShutdownEngineSubSystems();

    mock_application->Unload();

    OE_TRACE("Shutting down logger for unit test");
    other::Logger::Shutdown();
  }
      
  Ref<SceneRenderer> MockEngine::GetDefaultSceneRenderer(const uint32_t max_entities) const {
    const other::Path deferred_shader_path = shader_dir / "deferred_shading.oshader";

    uint32_t camera_binding_pnt = 0;
    std::vector<Uniform> cam_unis = {
      { "projection" , other::ValueType::MAT4 } ,
      { "view"       , other::ValueType::MAT4 } ,
      { "viewpoint"  , other::ValueType::VEC4 } ,
    };

    uint32_t model_binding_pnt = 1;
    std::vector<Uniform> model_unis = {
      { "models" , other::ValueType::MAT4 , max_entities } ,
    };
    
    uint32_t material_binding_pnt = 2;
    std::vector<Uniform> material_unis = {
      { "materials" , other::ValueType::USER_TYPE , max_entities , sizeof(other::Material) } ,
    };
    
    uint32_t light_binding_pnt = 3;
    std::vector<Uniform> light_unis = {
      { "num_lights" , other::ValueType::VEC4 } ,
      { "direction_lights" , other::ValueType::USER_TYPE , max_entities , sizeof(other::DirectionLight) } ,
      { "point_lights" , other::ValueType::USER_TYPE , max_entities , sizeof(other::PointLight) } ,
    };
    
    other::Layout default_layout = {
      { other::ValueType::VEC3 , "position" } ,
      { other::ValueType::VEC3 , "normal"   } ,
      { other::ValueType::VEC3 , "tangent"  } ,
      { other::ValueType::VEC3 , "binormal" } ,
      { other::ValueType::VEC2 , "uvs"      }
    };
    
    Ref<other::Shader> deferred_shader = other::BuildShader(deferred_shader_path);
    deferred_shader->Bind();
    deferred_shader->SetUniform("goe_position" , 0);
    deferred_shader->SetUniform("goe_normal" , 1);
    deferred_shader->SetUniform("goe_albedo" , 2);
    deferred_shader->Unbind();
      
    std::vector<float> fb_verts = {
       1.f ,  1.f , 1.f , 1.f ,
      -1.f ,  1.f , 0.f , 1.f ,
      -1.f , -1.f , 0.f , 0.f ,
       1.f , -1.f , 1.f , 0.f
    };   
    
    std::vector<uint32_t> fb_indices = { 
      0 , 1 , 3 , 
      1 , 2 , 3 
    };
    std::vector<uint32_t> fb_layout = { 
      2 , 2 
    };
    Ref<VertexArray> fb_mesh = NewRef<VertexArray>(fb_verts , fb_indices , fb_layout);

    SceneRenderSpec render_spec {
      .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
      .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt , other::SHADER_STORAGE) ,
      .pipelines = {
        {
          .lighting_shader = deferred_shader ,
          .target_mesh = fb_mesh ,
          .framebuffer_spec = {
            .depth_func = other::LESS_EQUAL ,
            .clear_color = { 0.1f , 0.1f , 0.1f , 1.f } ,
            .size = other::Renderer::WindowSize() ,
          } ,
          .vertex_layout = default_layout ,
          .model_uniforms = model_unis , 
          .model_binding_point = model_binding_pnt ,
          .material_uniforms = material_unis ,
          .material_binding_point = material_binding_pnt ,
          .debug_name = "Geometry" , 
        } ,
      } , 
    };
    return NewRef<SceneRenderer>(render_spec);
  }
      
  void MockEngine::InitializeEngineSubSystems() {
    IO::Initialize();
    EventQueue::Initialize(config);

    Renderer::Initialize(config);
    CHECKGL();

    UI::Initialize(config , Renderer::GetWindow());
    ScriptEngine::Initialize(config);

    PhysicsEngine::Initialize(config);
  }

  void MockEngine::ShutdownEngineSubSystems() {
    PhysicsEngine::Shutdown();

    ScriptEngine::Shutdown();

    UI::Shutdown();
    Renderer::Shutdown();
    EventQueue::Shutdown();
    IO::Shutdown();

    OE_INFO("Shutdown complete");
  }

  void EngineTest::EmptyEventLoop() const {
    other::IO::Update();
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      switch (e.type) {
        default:
          break;
      }
      ImGui_ImplSDL2_ProcessEvent(&e);
    }
    other::EventQueue::Clear();
  }

  Scope<App> NewApp(Engine* engine) {
    return NewScope<MockApp>(engine);
  }

} // namespace other
