/**
 * \file sandbox/main.cpp
 **/
#include "core/defines.hpp"

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <rendering/point_light.hpp>
#include <scene/environment.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/engine.hpp"
#include "core/filesystem.hpp"
#include "core/time.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "event/event_queue.hpp"

#include "scripting/script_engine.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/window.hpp"
#include "rendering/renderer.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/uniform.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/material.hpp"
#include "rendering/point_light.hpp"
#include "rendering/direction_light.hpp"
#include "rendering/geometry_pass.hpp"
#include "rendering/outline_pass.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

#include "sandbox_ui.hpp"

using other::FNV;

using other::Ref;
using other::NewRef;

using other::Scope;
using other::NewScope;

using other::Shader;
using other::Texture2D;
using other::VertexArray;
using other::CameraBase;
using other::PerspectiveCamera;
using other::Uniform;
using other::UniformBuffer;
using other::StaticModel;
using other::SceneRenderSpec;
using other::SceneRenderer;

void CheckGlError(int line);

class TestApp : public other::App {
  public:
    TestApp(other::Engine* engine) 
        : other::App(engine) {}
    virtual ~TestApp() override {}
};

#define CHECK() do { CheckGlError(__LINE__); } while (false)

void LaunchMock(const other::ConfigTable& config) {
  other::IO::Initialize();
  other::EventQueue::Initialize(config);
  other::Renderer::Initialize(config);
  other::UI::Initialize(config , other::Renderer::GetWindow());
  other::ScriptEngine::Initialize(config);
}

void ShutdownMock() {
  other::ScriptEngine::Shutdown();
  other::UI::Shutdown();
  other::Renderer::Shutdown();
  other::EventQueue::Shutdown();
  other::IO::Shutdown();
}

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

int main(int argc , char* argv[]) {
  try {
    other::CmdLine cmd_line(argc , argv);

    std::string config_path = "C:/Yock/code/OtherEngine/tests/sandbox/sandbox.other"; 
    other::IniFileParser parser(config_path);
    auto config = parser.Parse(); 

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Sandbox-Thread");

    /// for test reasons
    other::Engine mock_engine = other::Engine::Create(cmd_line , config , config_path);
    {
      Scope<other::App> app_handle = NewScope<TestApp>(&mock_engine);
      mock_engine.LoadApp(app_handle);
    }

    LaunchMock(config);
    OE_DEBUG("Sandbox Launched");

    mock_engine.PushCoreLayer();

    const other::Path shader_dir = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const other::Path default_path = shader_dir / "default.oshader";
    const other::Path normals_path = shader_dir / "normals.oshader";
    const other::Path fbshader_path = shader_dir / "fbshader.oshader";
    const other::Path add_fog_shader_path = shader_dir / "fog.oshader";
    const other::Path red_path = shader_dir / "red.oshader";
    const other::Path outline_path = shader_dir / "outline.oshader";
    const other::Path pure_geometry_path = shader_dir / "pure_geometry.oshader";
    
    SDL_SetRelativeMouseMode(SDL_TRUE);

    {
      auto win_size = other::Renderer::WindowSize();
      Ref<CameraBase> camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x , win_size.y });
      camera->SetPosition({ 0.f , 0.f , 3.f });

  #define FOGFB 0

      Ref<Shader> fbshader = other::BuildShader(fbshader_path);
#if FOGFB
      Ref<Shader> fog_shader = other::BuildShader(add_fog_shader_path);
#endif

      const other::Path editor_folder_path = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "textures" / "editor" / "folder.png";

      other::TextureSpecification tex_spec {
        .channels = other::RGBA ,
        .type = other::TEX_2D ,
        .filters = {
          .min = other::NEAREST ,
          .mag = other::NEAREST ,
        } ,
        .wrap = {
          .s_val = other::REPEAT , 
          .t_val = other::REPEAT , 
        } ,
        .name = editor_folder_path.filename().string() ,
      };
      Ref<other::Texture> icon = NewRef<Texture2D>(editor_folder_path , tex_spec);

      Scope<VertexArray> fb_mesh = NewScope<VertexArray>(fb_verts , fb_indices , fb_layout);

      other::AssetHandle model_handle = other::ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
      Ref<StaticModel> model = other::AssetManager::GetAsset<StaticModel>(model_handle);
    
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
      
      other::Layout default_layout = {
        { other::ValueType::VEC3 , "position" } ,
        { other::ValueType::VEC3 , "normal"   } ,
        { other::ValueType::VEC3 , "tangent"  } ,
        { other::ValueType::VEC3 , "binormal" } ,
        { other::ValueType::VEC2 , "uvs"      }
      };

      std::vector<Uniform> outline_unis = {
        { "outline_color" , other::VEC3 } ,
      };
      std::vector<Uniform> geometry_unis = {
      };
      Ref<other::Shader> outline_shader = other::BuildShader(outline_path);
      Ref<other::Shader> geometry_shader = other::BuildShader(default_path);
      
      Ref<other::RenderPass> geom_pass = NewRef<other::GeometryPass>(geometry_unis , geometry_shader);
      Ref<other::RenderPass> outline_pass = NewRef<other::OutlinePass>(outline_unis , outline_shader);


      SceneRenderSpec render_spec {
        .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
        .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt , other::SHADER_STORAGE) ,
        .passes = {
          {
            .name = "Draw Normals" , 
            .tag_col = { 0.f , 1.f , 0.f , 1.f } ,
            .uniforms = {
              { "magnitude" , other::FLOAT } ,
            } ,
            .shader = other::BuildShader(normals_path) ,
          } ,
          {
            .name = "Pure Geometry" , 
            .tag_col = { 0.f , 0.f , 0.f , 1.f } ,
            .uniforms = {} ,
            .shader = other::BuildShader(pure_geometry_path) ,
          } ,
        } ,
        .pipelines = {
          {
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
          {
            .framebuffer_spec = {
              .depth_func = other::LESS_EQUAL ,
              .clear_color = { 0.3f , 0.3f , 0.3f , 1.f } ,
              .size = other::Renderer::WindowSize() ,
            } ,
            .vertex_layout = default_layout ,
            .model_uniforms = model_unis , 
            .model_binding_point = model_binding_pnt ,
            .material_uniforms = material_unis ,
            .material_binding_point = material_binding_pnt ,
            .debug_name = "Debug" , 
          } ,
          {
            .framebuffer_spec = {
              .depth_func = other::LESS_EQUAL ,
              .clear_color = { 0.3f , 0.3f , 0.3f , 1.f } ,
              .size = other::Renderer::WindowSize() ,
            } ,
            .vertex_layout = default_layout ,
            .model_uniforms = model_unis , 
            .model_binding_point = model_binding_pnt ,
            .material_uniforms = material_unis ,
            .material_binding_point = material_binding_pnt ,
            .debug_name = "Outline" ,
          } ,
        } , 
        .ref_passes = {
          geom_pass , outline_pass ,
        } ,
        .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV(geom_pass->Name()) } } ,
          { FNV("Debug")    , { FNV("Pure Geometry")      , FNV("Draw Normals")  } } ,
          { FNV("Outline")  , { FNV(outline_pass->Name()) , FNV(geom_pass->Name()) } } ,
        } ,
      };
      Scope<SceneRenderer> renderer = NewScope<SceneRenderer>(render_spec);

      CHECK();

      OE_INFO("Running");

      other::time::DeltaTime dt;
      dt.Start();

      bool running = true;

      bool camera_lock = false;
      other::Mouse::LockCursor();
        
      CHECKGL();

      glm::vec3 cube_pos1 = glm::vec3(0.f , 0.f , 0.f);
      glm::mat4 cube_model1 = glm::mat4(1.f);
      float cube_rotation1 = 0.1f;
      other::Material cube_material1 = {
        .ambient  = { 1.0f , 0.5f , 0.31f , 1.f } ,
        .diffuse  = { 1.0f , 0.5f , 0.31f , 1.f } ,
        .specular = { 0.5f , 0.5f , 0.50f , 1.f } ,
        .shininess = 32.f ,
      };
      
      glm::vec3 cube_pos2 = glm::vec3(2.f , 0.f , 0.f);
      glm::mat4 cube_model2 = glm::mat4(1.f);
      other::Material cube_material2 = {
        .ambient  = { 1.0f , 0.31f , 0.5f , 1.f } ,
        .diffuse  = { 1.0f , 0.31f , 0.5f , 1.f } ,
        .specular = { 0.5f , 0.5f , 0.50f , 1.f } ,
        .shininess = 32.f ,
      };
      
      const glm::vec3 light_scale = glm::vec3(0.2f , 0.2f , 0.2f);

      glm::mat4 light_model1 = glm::mat4(1.f);
      glm::vec3 light_pos1 = glm::vec3(1.2f , 1.f , 2.f);
      light_model1 = glm::translate(light_model1 , light_pos1);
      light_model1 = glm::scale(light_model1 , light_scale);

      glm::mat4 light_model2 = glm::mat4(1.f);
      glm::vec3 light_pos2 = glm::vec3(-1.2f , 1.f , 2.f);
      light_model2 = glm::translate(light_model2 , light_pos2);
      light_model2 = glm::scale(light_model2 , light_scale);

      Ref<other::Environment> environment = NewRef<other::Environment>();
      environment->point_lights = {
        {
          .position = glm::vec4(light_pos1 , 1.f),
          .ambient  = { 0.2f , 0.2f , 0.2f , 1.f } ,
          .diffuse  = { 0.5f , 0.5f , 0.5f , 1.f } ,
          .specular = { 1.0f , 1.0f , 1.0f , 1.f } ,
        } ,
        {
          .position = glm::vec4(light_pos2 , 1.f),
          .ambient  = { 0.5f , 0.5f , 0.5f , 1.f } ,
          .diffuse  = { 0.2f , 0.2f , 0.2f , 1.f } ,
          .specular = { 1.0f , 1.0f , 1.0f , 1.f } ,
        } ,
      };
      environment->direction_lights = {
        {
          .direction = { -0.2f , -1.f , -0.3f , 1.f } ,
          .ambient  = { 0.2f , 0.2f , 0.2f , 1.f } ,
          .diffuse  = { 0.5f , 0.5f , 0.5f , 1.f } ,
          .specular = { 1.0f , 1.0f , 1.0f , 1.f } ,
        } ,
      };
      

      other::Material light_material = {
        .ambient  = { 1.f , 1.f , 1.f  , 1.f } ,
        .diffuse  = { 1.f , 1.f , 1.f  , 1.f } ,
        .specular = { 1.f , 1.f , 1.f  , 1.f } ,
        .shininess = 1.f ,
      };

      glm::vec3 outline_color{ 1.f , 0.f , 0.f };

      while (running) {
        other::IO::Update();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
          switch (event.type) {
            case SDL_QUIT: running = false; break;
            case SDL_WINDOWEVENT:
              switch (event.window.event) {
                case SDL_WINDOWEVENT_CLOSE: running = false; break; 
                default: break;
              }
            break;
            case SDL_KEYDOWN:
              switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_c: 
                  camera_lock = !camera_lock;
                  if (camera_lock) {
                    other::Mouse::FreeCursor();
                  } else {
                    other::Mouse::LockCursor();
                  }
                break;
                default: break;
              }
            break;
            default:
              break;
          }

          ImGui_ImplSDL2_ProcessEvent(&event);
        }

        /// dont want event queue to actually dispatch events, so we just clear the buffer to avoid an
        ///   overflow of the event queue's buffer
        other::EventQueue::Clear();

        if (!camera_lock) {
          other::DefaultUpdateCamera(camera);
        }
        
        cube_model1 = glm::mat4(1.f);
        cube_model1 = glm::translate(cube_model1 , cube_pos1);
        cube_model1 = glm::rotate(cube_model1 , cube_rotation1 , { 1.f , 1.f , 1.f });
        cube_rotation1 += 0.1f;
        
        cube_model2 = glm::mat4(1.f);
        cube_model2 = glm::translate(cube_model2 , cube_pos2);

        light_model1 = glm::mat4(1.f);
        light_model1 = glm::translate(light_model1 , light_pos1);
        light_model1 = glm::scale(light_model1 , light_scale);
        
        light_model2 = glm::mat4(1.f);
        light_model2 = glm::translate(light_model2 , light_pos2);
        light_model2 = glm::scale(light_model2 , light_scale);

        other::Renderer::GetWindow()->Clear();

        renderer->BeginScene(camera , environment);

        renderer->SetUniform("Draw Normals" , "magnitude" , 0.4f);
        renderer->SetUniform(outline_pass->Name() , "outline_color" , outline_color);

        /// outline the first cube but not the second
        renderer->SubmitStaticModel("Outline" , model , cube_model1 , cube_material1);

        renderer->SubmitStaticModel("Geometry" , model , cube_model1 , cube_material1);
        renderer->SubmitStaticModel("Geometry" , model , cube_model2 , cube_material2);

        /// light cube only gets rendered in debug
        renderer->SubmitStaticModel("Debug" , model , cube_model1 , cube_material1);
        renderer->SubmitStaticModel("Debug" , model , cube_model2 , cube_material2);
        renderer->SubmitStaticModel("Debug" , model , light_model1 , light_material);
        renderer->SubmitStaticModel("Debug" , model , light_model2 , light_material);
        renderer->EndScene();

        const auto& frames = renderer->GetRender(); 
        const auto& vp = frames.at(FNV("Geometry"));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D , vp->texture);
        fbshader->SetUniform("screen_tex" , 0);
        fb_mesh->Draw(other::TRIANGLES);

#define UI_ENABLED 1
#if UI_ENABLED
        other::UI::BeginFrame();
        const ImVec2 win_size = { (float)other::Renderer::WindowSize().x , (float)other::Renderer::WindowSize().y };
        if (ImGui::Begin("Frames")) {
          RenderItem(frames.at(FNV("Debug"))->texture , "Debug" , win_size);
          RenderItem(frames.at(FNV("Outline"))->texture , "Outline" , win_size);
        } 
        ImGui::End();

        if (ImGui::Begin("Render Settings")) {
          bool edited = false;
          other::ui::Underline();

          ImGui::Text("Debug Controls");
          ImGui::Separator();
          other::ui::widgets::DrawVec3Control("outline color" , outline_color , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                              { 0.f , 0.f , 0.f } , { 1.f , 1.f , 1.f } , 0.1f);
          ImGui::Separator();

          ImGui::Text("Cube Controls");
          ImGui::Separator();
          other::ui::widgets::DrawVec3Control("cube1 position" , cube_pos1 , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                              { -100.f , -100.f , -100.f } , { 100.f , 100.f , 100.f } , 0.5f); 
          RenderMaterial("cube1 material" , cube_material1);
          other::ui::widgets::DrawVec3Control("cube2 position" , cube_pos2 , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                              { -100.f , -100.f , -100.f } , { 100.f , 100.f , 100.f } , 0.5f); 
          RenderMaterial("cube2 material" , cube_material2);
          ImGui::Separator();

          ImGui::Text("Light Controls");
          ImGui::Separator();
          RenderPointLight("point light 1" , environment->point_lights[0]);
          RenderPointLight("point light 2" , environment->point_lights[1]);
          RenderDirectionLight("direction light" , environment->direction_lights[0]);
          ImGui::Separator();
        }
        ImGui::End();
        other::UI::EndFrame();
#endif // !UI_ENABLED

        other::Renderer::GetWindow()->SwapBuffers();
      }
    }

    ShutdownMock();

    OE_INFO("Succesful exit");
    
    other::Logger::Shutdown();
    return 0;
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
  } catch (const other::ShaderException& e) {
    std::cout << "caught shader error : " << e.what() << "\n";
  } catch(const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
  } catch (...) {
    std::cout << "unknown error" << "\n";
    return 1;
  }
}

void CheckGlError(int line) {
  GLenum err = glGetError();
  while (err != GL_NO_ERROR) {
    switch (err) {
      case GL_INVALID_ENUM: 
        OE_ERROR("OpenGL Error INVALID_ENUM : {}" , line);
      break;
      case GL_INVALID_VALUE:
        OE_ERROR("OpenGL Error INVALID_VALUE : {}" , line);
      break;
      case GL_INVALID_OPERATION:
        OE_ERROR("OpenGL Error INVALID_OPERATION : {}" , line);
      break;
      case GL_STACK_OVERFLOW:
        OE_ERROR("OpenGL Error STACK_OVERFLOW : {}" , line);
      break;
      case GL_STACK_UNDERFLOW:
        OE_ERROR("OpenGL Error STACK_UNDERFLOW : {}" , line);
      break;
      case GL_OUT_OF_MEMORY:
        OE_ERROR("OpenGL Error OUT_OF_MEMORY : {}" , line);
      break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        OE_ERROR("OpenGL Error INVALID_FRAMEBUFFER_OPERATION : {}" , line);
      break;
      case GL_CONTEXT_LOST:
        OE_ERROR("OpenGL Error CONTEXT_LOST : {}" , line);
      break;
      case GL_TABLE_TOO_LARGE:
        OE_ERROR("OpenGL Error TABLE_TOO_LARGE : {}" , line);
      break;
      default: break;
    }
    err = glGetError();
  }

}
  
