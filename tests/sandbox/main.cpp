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
#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/uniform.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/ui/ui.hpp"

using other::FNV;

using other::Ref;
using other::NewRef;

using other::Scope;
using other::NewScope;

using other::Shader;
using other::VertexArray;
using other::CameraBase;
using other::PerspectiveCamera;
using other::Uniform;
using other::UniformBuffer;
using other::ModelSource;
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

std::vector<uint32_t> fb_indices{ 0 , 1 , 3 , 1 , 2 , 3 };
std::vector<uint32_t> fb_layout{ 2 , 2 };

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
    
    const other::Path shader1_path = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "default.oshader";
    const other::Path shader2_path = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "normals.oshader";
    const other::Path fbshader_path = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "fbshader.oshader";
    
    SDL_SetRelativeMouseMode(SDL_TRUE);

    {
      auto win_size = other::Renderer::WindowSize();
      Ref<CameraBase> camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x , win_size.y });
      camera->SetPosition({ 0.f , 0.f , 3.f });

      Ref<Shader> fbshader = other::BuildShader(fbshader_path);
      Scope<VertexArray> fb_mesh = NewScope<VertexArray>(fb_verts , fb_indices , fb_layout);

      glm::mat4 model1 = glm::mat4(1.f);
      glm::mat4 model2 = glm::mat4(1.f); // glm::translate(model1 , glm::vec3(0.3f , 0.1f , 0.0f));

      other::AssetHandle model_handle = other::ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
      Ref<StaticModel> model = other::AssetManager::GetAsset<StaticModel>(model_handle);
      Ref<ModelSource> model_source = model->GetModelSource();
    
      uint32_t camera_binding_pnt = 0;
      std::vector<Uniform> cam_unis = {
        { "projection" , other::ValueType::MAT4 } ,
        { "view"       , other::ValueType::MAT4 }
      };
    
      uint32_t model_binding_pnt = 1;
      std::vector<Uniform> model_unis = {
        { "models" , other::ValueType::MAT4 , 100 } ,
      };
        
      SceneRenderSpec render_spec {
        .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
        .model_storage = NewRef<UniformBuffer>("ModelData" , model_unis , model_binding_pnt , other::SHADER_STORAGE) ,
        .passes = {
          {
            .name = "Draw Geometry" , 
            .tag_col = { 1.f , 0.f , 0.f , 1.f } ,
            .uniforms = {} ,
            .shader = other::BuildShader(shader1_path) ,
          } ,
          {
            .name = "Draw Normals" , 
            .tag_col = { 1.f , 0.f , 0.f , 1.f } ,
            .uniforms = {} ,
            .shader = other::BuildShader(shader2_path) ,
          } ,
        } ,
        .pipelines = {
          {
            .has_indices = true ,
            .buffer_cap = 4096 * sizeof(float) ,
            .framebuffer_spec = {
              .depth_func = other::LESS_EQUAL ,
              .clear_color = { 0.1f , 0.3f , 0.5f , 1.f } ,
              .size = other::Renderer::WindowSize() ,
            } ,
            .vertex_layout = {
              { other::ValueType::VEC3 , "position" } ,
              { other::ValueType::VEC3 , "normal"   } ,
              { other::ValueType::VEC3 , "tangent"  } ,
              { other::ValueType::VEC3 , "binormal" } ,
              { other::ValueType::VEC2 , "uvs"      }
            } ,
            .debug_name = "Geometry" , 
          } ,
          {
            .has_indices = true ,
            .buffer_cap = 4096 * sizeof(float) ,
            .framebuffer_spec = {
              .depth_func = other::LESS_EQUAL ,
              .clear_color = { 0.1f , 0.3f , 0.5f , 1.f } ,
              .size = other::Renderer::WindowSize() ,
            } ,
            .vertex_layout = {
              { other::ValueType::VEC3 , "position" } ,
              { other::ValueType::VEC3 , "normal"   } ,
              { other::ValueType::VEC3 , "tangent"  } ,
              { other::ValueType::VEC3 , "binormal" } ,
              { other::ValueType::VEC2 , "uvs"      }
            } ,
            .debug_name = "Debug" , 
          } ,
        } , 
        .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV("Draw Geometry") } } ,
          { FNV("Debug")    , { FNV("Draw Geometry") , FNV("Draw Normals") } }
        } ,
      };
      Scope<SceneRenderer> renderer = NewScope<SceneRenderer>(render_spec);

      CHECK();

      OE_INFO("Running");

      other::time::DeltaTime dt;
      dt.Start();

      bool running = true;

      bool camera_free = true;
      other::Mouse::LockCursor();

      while (running) {
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
                  if (!camera_free) {
                    other::Mouse::LockCursor();
                    camera_free = true;
                  } else {
                    other::Mouse::FreeCursor();
                    camera_free = false;
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

        if (camera_free) {
          other::DefaultUpdateCamera(camera);
        }

        model1 = glm::rotate(model1 , glm::radians(5.f) , { 1.f , 1.f , 0.f });

        other::Renderer::GetWindow()->Clear();

        renderer->BeginScene(camera);
        renderer->SubmitStaticModel(model , model1);
        renderer->EndScene();

#if 0
        auto& frame = renderer->GetRender().at(FNV("Geometry"));

        fbshader->Bind();
        fbshader->SetUniform("screen_tex" , 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D , frame->texture);

        fb_mesh->Draw(other::TRIANGLES);
#else
        other::UI::BeginFrame();

        auto RenderFrame = [](Ref<other::Framebuffer>& fb , const std::string& title) {
          ImGui::PushID(("##" + title).c_str());
          ImGui::Text("%s" , title.c_str());
          ImGui::SameLine();

          glm::ivec2 size = other::Renderer::WindowSize();
          ImGui::Image((void*)(uintptr_t)fb->texture , ImVec2(size.x , size.y) , ImVec2(0 , 1) , ImVec2(1 , 0));
          ImGui::PopID();
        };

        if (ImGui::Begin("Frames")) {
          auto frames = renderer->GetRender(); 
          RenderFrame(frames[FNV("Geometry")] , "Pipeline 1");
          RenderFrame(frames[FNV("Debug")]    , "Pipeline 2");
        } 
        ImGui::End();
        
        other::UI::EndFrame();
#endif

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
  
