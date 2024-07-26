/**
 * \file sandbox/main.cpp
 **/
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
#include "core/filesystem.hpp"
#include "core/time.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"
#include "input/keyboard.hpp"

#include "application/app.hpp"
#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/uniform.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/scene_renderer.hpp"

#include "common/test_app.hpp"
#include "common/engine_mock.hpp"

using other::Shader;
using other::Texture2D;
using other::VertexArray;
using other::CameraBase;
using other::PerspectiveCamera;
using other::Uniform;
using other::UniformBuffer;
using other::ModelSource;
using other::StaticModel;
using other::SceneRenderSpec;
using other::SceneRenderer;

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
    const other::Path add_fog_shader_path = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "fog.oshader";
    
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

      glm::mat4 model1 = glm::mat4(1.f);
      glm::mat4 model2 = glm::mat4(1.f); 

      other::AssetHandle model_handle = other::ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
      Ref<StaticModel> model = other::AssetManager::GetAsset<StaticModel>(model_handle);
      Ref<ModelSource> model_source = model->GetModelSource();
      
      other::AssetHandle floor_handle = other::ModelFactory::CreateBox({ 1.f , 1.f , 1.f });
      Ref<StaticModel> floor = other::AssetManager::GetAsset<StaticModel>(floor_handle);
      Ref<ModelSource> floor_source = floor->GetModelSource();
    
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
        
      SceneRenderSpec render_spec {
        .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
        .passes = {
          {
            .name = "Draw Geometry" , 
            .tag_col = { 1.f , 0.f , 0.f , 1.f } ,
            .uniforms = {} ,
            .shader = other::BuildShader(shader1_path) ,
          } ,
          {
            .name = "Draw Normals" , 
            .tag_col = { 0.f , 1.f , 0.f , 1.f } ,
            .uniforms = {
              { "magnitude" , other::FLOAT } ,
            } ,
            .shader = other::BuildShader(shader2_path) ,
          } ,
#if FOGFB
          {
            .name = "Add Fog" , 
            .tag_col = { 0.f , 0.f , 1.f , 1.f } ,
            .uniforms = {
              { "depth_tex" , other::SAMPLER2D } ,
              { "screen_tex" , other::SAMPLER2D } ,
            } ,
            .shader = other::BuildShader(add_fog_shader_path) ,
          } ,
#endif
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
            .model_storage = NewRef<UniformBuffer>("ModelData" , model_unis , model_binding_pnt , other::SHADER_STORAGE) ,
            .debug_name = "Debug" , 
          } ,
        } , 
        .pipeline_to_pass_map = {
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
        
      CHECKGL();

      bool rendering_floor = false;
      model2 = glm::translate(glm::mat4(1.f) , glm::vec3(0 , -2.f , 0)) *
               glm::scale(glm::mat4(1.f) , glm::vec3(10 , 1 , 10));

      while (running) {
        other::Mouse::Update();
        other::Keyboard::Update();

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
                case SDLK_f: rendering_floor = !rendering_floor; break;
                case SDLK_c: 
                  camera_free = !camera_free;
                  if (!camera_free) {
                    other::Mouse::LockCursor();
                  } else {
                    other::Mouse::FreeCursor();
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
        renderer->SetUniform("Draw Normals" , "magnitude" , 0.4f);
        renderer->SubmitStaticModel(model , model1);
        if (rendering_floor) {
          renderer->SubmitStaticModel(floor , model2);
        }
        renderer->EndScene();

        const auto& frames = renderer->GetRender(); 

        const auto& vp = frames.at(FNV("Debug"));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D , vp->texture);
        
        fbshader->SetUniform("screen_tex" , 0);
        fb_mesh->Draw(other::TRIANGLES);

#if FOGFB
        fog_shader->SetUniform("screen_tex" , 0);
        fog_shader->SetUniform("depth_tex" , 1);
        fb_mesh->Draw(other::TRIANGLES);
#endif

        other::UI::BeginFrame();

        auto RenderFrame = [](const Ref<other::Framebuffer>& fb , const std::string& title , ImVec2 size = ImVec2(800 , 600)) {
          ImGui::PushID(("##" + title).c_str());
          ImGui::Text("%s" , title.c_str());
          ImGui::SameLine();

          ImGui::Image((void*)(uintptr_t)fb->texture , ImVec2(size.x , size.y) , ImVec2(0 , 1) , ImVec2(1 , 0));
          ImGui::PopID();
        };

        auto RenderTexture = [](const Ref<other::Texture>& texture , const std::string& title , ImVec2 size = ImVec2(800 , 600)) {
          ImGui::PushID(("##" + title).c_str());
          ImGui::Text("%s" , title.c_str());
          ImGui::SameLine();

          ImGui::Image((void*)(uintptr_t)texture->GetRendererId() , ImVec2(size.x , size.y) , ImVec2(0 , 1) , ImVec2(1 , 0));
          ImGui::PopID();
        };

        if (ImGui::Begin("Frames")) {
          ImVec2 win_size = { (float)other::Renderer::WindowSize().x , (float)other::Renderer::WindowSize().y };
          RenderFrame(frames.at(FNV("Debug"))    , "Debug" , win_size);

          // RenderTexture(icon , "Folder Icon");
        } 
        ImGui::End();

        
        other::UI::EndFrame();

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
