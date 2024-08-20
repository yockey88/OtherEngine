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

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/engine.hpp"
#include "core/filesystem.hpp"
#include "core/time.hpp"
#include "core/uuid.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "event/event_queue.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/mesh.hpp"

#include "scene/scene_serializer.hpp"
#include "scene/environment.hpp"
#include "scene/scene.hpp"

#include "scripting/script_engine.hpp"
#include "physics/phyics_engine.hpp"
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
#include "rendering/pipeline.hpp"
#include "rendering/point_light.hpp"
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

using other::UUID;

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

using other::SceneSerializer;

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
  other::ScriptEngine::Initialize(config);
  other::PhysicsEngine::Initialize(config);
  
  other::Renderer::Initialize(config);
  other::UI::Initialize(config , other::Renderer::GetWindow());
}

void ShutdownMock() {
  other::UI::Shutdown();
  other::Renderer::Shutdown();

  other::PhysicsEngine::Shutdown();
  other::ScriptEngine::Shutdown();
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

    const other::Path sandbox_dir = "C:/Yock/code/OtherEngine/tests/sandbox";

    const other::Path config_path = sandbox_dir / "sandbox.other"; 
    other::IniFileParser parser(config_path.string());
    auto config = parser.Parse(); 

    other::Logger::Open(config);
    other::Logger::Instance()->RegisterThread("Sandbox-Thread");

    /// for test reasons
    other::Engine mock_engine = other::Engine::Create(cmd_line , config , config_path.string());
    {
      Scope<other::App> app_handle = NewScope<TestApp>(&mock_engine);
      mock_engine.LoadApp(app_handle);
    }

    LaunchMock(config);
    OE_DEBUG("Sandbox Launched");

    mock_engine.PushCoreLayer();
    other::ScriptEngine::LoadProjectModules();


    const other::Path engine_core_dir = other::Filesystem::GetEngineCoreDir();
    const other::Path assets_dir = engine_core_dir / "OtherEngine" / "assets";

    const other::Path shader_dir = assets_dir / "shaders";
      const other::Path default_path = shader_dir / "default.oshader";
      const other::Path normals_path = shader_dir / "normals.oshader";
      const other::Path fbshader_path = shader_dir / "fbshader.oshader";
      const other::Path deferred_shader_path = shader_dir / "deferred_shading.oshader";
      const other::Path add_fog_shader_path = shader_dir / "fog.oshader";
      const other::Path red_path = shader_dir / "red.oshader";
      const other::Path outline_path = shader_dir / "outline.oshader";
      const other::Path pure_geometry_path = shader_dir / "pure_geometry.oshader";
      
    const other::Path texture_dir = assets_dir / "textures";
      const other::Path editor_texture_dir = texture_dir / "editor";
        const other::Path editor_folder_path = editor_texture_dir / "folder.png";
    
    const other::Path scene_dir = assets_dir / "scenes";
      
    const other::Path scenepath = sandbox_dir / "test_scene.yscn";
    
    const other::Path bin_dir = engine_core_dir / "bin";
    const other::Path debug_bin_dir = bin_dir / "Debug";
    
    SDL_SetRelativeMouseMode(SDL_TRUE);

    {
      auto win_size = other::Renderer::WindowSize();
      Ref<CameraBase> camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x , win_size.y });
      camera->SetPosition({ 0.f , 0.f , 3.f });

      Ref<Shader> fbshader = other::BuildShader(fbshader_path);

      Scope<VertexArray> fb_mesh = NewScope<VertexArray>(fb_verts , fb_indices , fb_layout);
    
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
      Ref<other::Shader> deferred_shader = other::BuildShader(deferred_shader_path);
      deferred_shader->Bind();
      deferred_shader->SetUniform("goe_position" , 0);
      deferred_shader->SetUniform("goe_normal" , 1);
      deferred_shader->SetUniform("goe_albedo" , 2);
      deferred_shader->Unbind();
        
      other::RenderPassSpec normal_pass_spec {
        .name = "Draw Normals" , 
        .tag_col = { 0.f , 1.f , 0.f , 1.f } ,
        .uniforms = {
          { "magnitude" , other::FLOAT } ,
        } ,
        .shader = other::BuildShader(normals_path) ,
      };
      Ref<other::RenderPass> normal_pass = NewRef<other::RenderPass>(normal_pass_spec);
      Ref<other::RenderPass> geom_pass = NewRef<other::GeometryPass>(geometry_unis , geometry_shader);
      Ref<other::RenderPass> outline_pass = NewRef<other::OutlinePass>(outline_unis , outline_shader);

      SceneRenderSpec render_spec {
        .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
        .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt , other::SHADER_STORAGE) ,
        .passes = {
          {
            .name = "Albedo" , 
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
            .debug_name = "Outline" ,
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
            .debug_name = "Normals" , 
          } ,
        } , 
        .ref_passes = {
          geom_pass , outline_pass , normal_pass ,
        } ,
        .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV(geom_pass->Name()) } } ,
          { FNV("Normals")  , { FNV("Albedo")             , FNV(normal_pass->Name())  } } ,
          { FNV("Outline")  , { FNV(outline_pass->Name()) , FNV("Albedo") } } ,
        } ,
      };
      Ref<SceneRenderer> renderer = NewRef<SceneRenderer>(render_spec);

      CHECK();

      OE_INFO("Running");

      other::time::DeltaTime dt;
      dt.Start();

      bool running = true;

      bool camera_lock = false;
      other::Mouse::LockCursor();
        
      CHECKGL();

      glm::vec3 cube_pos1 = glm::vec3(0.f , 0.f , 0.f);
      other::Material cube_material1 = {
        .ambient  = { 1.0f , 0.5f , 0.31f , 1.f } ,
        .diffuse  = { 1.0f , 0.5f , 0.31f , 1.f } ,
        .specular = { 0.5f , 0.5f , 0.50f , 1.f } ,
        .shininess = 32.f ,
      };
      
      glm::vec3 cube_pos2 = glm::vec3(2.f , 0.f , 0.f);
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

      glm::vec3 outline_color{ 1.f , 0.f , 0.f };

      UUID id = FNV(scenepath.string());
      SceneSerializer serializer;
      Ref<other::Scene> scene = nullptr;

      {
        auto loaded_scene = serializer.Deserialize(scenepath.string());
        if (loaded_scene.scene == nullptr) {
          OE_ERROR("Failed to deserialize scene : {}" , scenepath.string());
          throw std::runtime_error("Failed to deserialize scene!");
        }

        scene = loaded_scene.scene;
      }

      other::ScriptEngine::SetSceneContext(scene);
      other::Renderer::SetSceneContext(scene);

      scene->Initialize();
      scene->Start();

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
                case SDLK_s: 
                  // if scene started stop scene else start scene
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
        
        auto environment = scene->GetEnvironment();

        light_model1 = glm::mat4(1.f);
        light_model1 = glm::translate(light_model1 , glm::vec3(environment->point_lights[0].position));
        light_model1 = glm::scale(light_model1 , light_scale);
        
        light_model2 = glm::mat4(1.f);
        light_model2 = glm::translate(light_model2 , glm::vec3(environment->point_lights[1].position));
        light_model2 = glm::scale(light_model2 , light_scale);

        scene->EarlyUpdate(0.16f);
        scene->Update(0.16f);
        scene->LateUpdate(0.16f);

        other::Renderer::GetWindow()->Clear();

        renderer->BeginScene(camera , environment);
        scene->Render(renderer);
        renderer->EndScene();

        const auto& frames = renderer->GetRender(); 
        const auto& vp = frames.at(FNV("Geometry"));
        
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D , vp->texture);
        deferred_shader->Bind();
        fb_mesh->Draw(other::TRIANGLES);

#define UI_ENABLED 1
#if UI_ENABLED
        other::UI::BeginFrame();
        const ImVec2 win_size = { (float)other::Renderer::WindowSize().x , (float)other::Renderer::WindowSize().y };
        if (ImGui::Begin("Frames")) {
          RenderItem(frames.at(FNV("Normals"))->texture , "Normals" , win_size);
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

          ImGui::Text("Scene Controls");
          auto& reg = scene->Registry();

          ImGui::Text("Transforms =====");
          reg.view<other::Tag , other::Transform>().each([&](other::Tag& tag , other::Transform& transform) {
            ImGui::PushID((tag.name + "##transform-widget").c_str());
            if (other::ui::widgets::DrawVec3Control(other::fmtstr("{} position", tag.name) , 
                                                    transform.position , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                                    { -100.f , -100.f , -100.f } , { 100.f , 100.f , 100.f } , 0.5f)) {}
            ImGui::Separator();
            ImGui::PopID();
          });
          
          ImGui::Text("Materials =====");
          reg.view<other::Tag , other::StaticMesh>().each([&](other::Tag& tag , other::StaticMesh& mesh) {
            ImGui::PushID((tag.name + "##static-mesh-widget").c_str());
            RenderMaterial(other::fmtstr("{} material" , tag.name) , mesh.material);
            ImGui::Separator();
            ImGui::PopID();
          });
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
    
      scene->Stop();
      scene->Shutdown();
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
  
