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
#include <rendering/model_factory.hpp>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/filesystem.hpp"
#include "core/time.hpp"
#include "core/uuid.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "application/app.hpp"

#include "event/event_queue.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/camera.hpp"

#include "scene/scene_serializer.hpp"
#include "scene/environment.hpp"
#include "scene/scene.hpp"

#include "scripting/script_engine.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/shader.hpp"
#include "rendering/texture.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/uniform.hpp"
#include "rendering/model.hpp"
#include "rendering/material.hpp"
#include "rendering/point_light.hpp"
#include "rendering/direction_light.hpp"
#include "rendering/geometry_pass.hpp"
#include "rendering/outline_pass.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/point_light.hpp"
#include "rendering/scene_renderer.hpp"

#include "common/test_app.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"

#include "mock_app.hpp"
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
    const other::Path sandbox_dir = "C:/Yock/code/OtherEngine/tests/sandbox";
    const other::Path config_path = sandbox_dir / "sandbox.other"; 

    other::CmdLine cmd_line(argc , argv);
    cmd_line.SetFlag("--project" , { config_path.string() });

    /// for test reasons
    other::Engine mock_engine(cmd_line); 
    other::Logger::Open(mock_engine.config);
    other::Logger::Instance()->RegisterThread("Sandbox Thread");

    mock_engine.LoadApp();
    OE_DEBUG("Sandbox Launched");

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

      Ref<VertexArray> fb_mesh = NewRef<VertexArray>(fb_verts , fb_indices , fb_layout);
      Ref<other::Framebuffer> framebuffer = NewRef<other::Framebuffer>(other::FramebufferSpec{
        .size = win_size ,
      });
    
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
        { "point_lights" , other::ValueType::USER_TYPE , 100 , sizeof(other::PointLight) } ,
        { "direction_lights" , other::ValueType::USER_TYPE , 100 , sizeof(other::DirectionLight) } ,
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
        
      other::RenderPassSpec normal_pass_spec {
        .name = "Draw Normals" , 
        .tag_col = { 0.f , 1.f , 0.f , 1.f } ,
        .uniforms = {
          { "magnitude" , other::FLOAT } ,
        } ,
        .shader = other::BuildShader(normals_path) ,
      };
      Ref<other::RenderPass> normal_pass = NewRef<other::RenderPass>(normal_pass_spec);
      normal_pass->SetInput("magnitude" , 0.2f);
      
      other::RenderPassSpec pure_geom_pass_spec {
        .name = "Pure Geometry" , 
        .tag_col = { 0.f , 0.f , 1.f , 1.f } ,
        .uniforms = {
        } ,
        .shader = other::BuildShader(pure_geometry_path) ,
      };
      Ref<other::RenderPass> pure_geom_pass = NewRef<other::RenderPass>(pure_geom_pass_spec);

      Ref<other::RenderPass> geom_pass = NewRef<other::GeometryPass>(geometry_unis , geometry_shader);
      Ref<other::RenderPass> outline_pass = NewRef<other::OutlinePass>(outline_unis , outline_shader);

      SceneRenderSpec render_spec {
        .camera_uniforms = NewRef<UniformBuffer>("Camera" , cam_unis , camera_binding_pnt) ,
        .light_uniforms = NewRef<UniformBuffer>("Lights" , light_unis , light_binding_pnt , other::SHADER_STORAGE) ,
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
              .clear_color = { 0.1f , 0.1f , 0.1f , 1.f } ,
              .size = other::Renderer::WindowSize() ,
            } ,
            .vertex_layout = default_layout ,
            .model_uniforms = model_unis , 
            .model_binding_point = model_binding_pnt ,
            .material_uniforms = material_unis ,
            .material_binding_point = material_binding_pnt ,
            .debug_name = "Debug" ,
          } ,
        } , 
        .passes = {
          geom_pass , normal_pass , pure_geom_pass ,
        } ,
        .pipeline_to_pass_map = {
          { FNV("Geometry") , { FNV(geom_pass->Name()) } } ,
          { FNV("Debug") , { FNV(geom_pass->Name()) , FNV(normal_pass->Name()) } } ,
        } ,
      };
      Ref<SceneRenderer> renderer = NewRef<SceneRenderer>(render_spec);

      bool running = true;
      bool camera_lock = true;

      if (camera_lock) {
        other::Mouse::FreeCursor();
      } else {
        other::Mouse::LockCursor();
      }
        
      other::Material cube_material1 = {
        .color = { 1.0f , 0.5f , 0.31f , 1.f } ,
        .shininess = 32.f ,
      };
      
      other::Material cube_material2 = {
        .color = { 0.1f , 0.5f , 0.31f , 1.f } ,
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

      {
        auto* cube = scene->GetEntity("cube");
        auto& cube_mesh = cube->GetComponent<other::StaticMesh>();
        cube_mesh.material = cube_material1;
        
        auto* floor = scene->GetEntity("floor");
        auto& floor_mesh = floor->GetComponent<other::StaticMesh>();
        floor_mesh.material = cube_material2;

        auto* sun = scene->GetEntity("sun");
        auto& sun_l = sun->GetComponent<other::LightSource>();
        sun_l.direction_light = {
          .direction = { 0.f , -1.f , 0.f , 1.f } ,
          .color = { 0.22f , 0.22f , 0.11f , 1.f } ,
        };

        auto* cam_ent = scene->CreateEntity("camera");
        auto& camera_comp = cam_ent->AddComponent<other::Camera>();
        camera_comp.is_primary = true;
        camera_comp.camera = NewRef<other::PerspectiveCamera>(other::Renderer::GetWindow()->Size());
        camera_comp.camera->SetPosition({ 0.f , 0.f , 3.f });
        
        auto* point_light = scene->GetEntity("plight");
        // auto& point_light_comp = point_light->GetComponent<other::LightSource>();
        // point_light_comp.pointlight = {
        //   .position = { 1.f , 4.f , 1.f , 1.f } ,
        //   .color = { 1.f , 0.2f , 0.2f , 1.f } ,
        // };
      }

      other::ScriptEngine::SetSceneContext(scene);
      other::Renderer::SetSceneContext(scene);

      scene->Initialize();
      scene->Start();
      other::DefaultUpdateCamera(camera);

      OE_DEBUG("Lights [{} , {}]" , scene->GetEnvironment()->direction_lights.size() ,
                                    scene->GetEnvironment()->point_lights.size());
      
      OE_INFO("Running");

      bool render_to_window = true;

      other::time::DeltaTime dt;
      dt.Start();
      while (running) {
        float delta = dt.Get();
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
                case SDLK_r:
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

        other::EventQueue::Clear();

        if (!camera_lock) {
          other::DefaultUpdateCamera(camera);
        }
        
        scene->EarlyUpdate(delta);
        scene->Update(delta);
        scene->LateUpdate(delta);

        other::Renderer::GetWindow()->Clear();

        // renderer->SubmitCamera(camera);
        scene->Render(renderer);
        renderer->EndScene();

        const auto& frames = renderer->GetRender(); 
        
        auto itr = frames.find(FNV("Geometry"));
        if (itr != frames.end()) {
          const auto& vp = frames.at(FNV("Geometry"));
          if (render_to_window) {
            other::Renderer::DrawFramebufferToWindow(vp);
          } else {
          }
        }


#define UI_ENABLED 1
#if UI_ENABLED
        other::UI::BeginFrame();
        const ImVec2 win_size = { (float)other::Renderer::WindowSize().x , (float)other::Renderer::WindowSize().y };

        if (ImGui::Begin("Frames")) {
          if (auto frame = frames.find(FNV("Debug")); frame != frames.end()) {
            RenderItem(frame->second->texture, "Debug", ImVec2(win_size.x, win_size.y));
          }
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

          ImGui::Text("===== Scene Controls =====");
          auto& reg = scene->Registry();

          ImGui::Text(" - Transforms =====");
          reg.view<other::Tag , other::Transform>().each([&](other::Tag& tag , other::Transform& transform) {
            ImGui::PushID((tag.name + "##transform-widget").c_str());
            if (other::ui::widgets::DrawVec3Control(other::fmtstr("{} position", tag.name) , 
                                                    transform.position , edited , 0.f , 100.f , other::ui::VectorAxis::ZERO ,
                                                    { -100.f , -100.f , -100.f } , { 100.f , 100.f , 100.f } , 0.5f)) {}
            ImGui::Separator();
            ImGui::PopID();
          });
          
          ImGui::Text(" - Materials =====");

          reg.view<other::Tag , other::StaticMesh>().each([&](other::Tag& tag , other::StaticMesh& mesh) {
            ImGui::PushID((tag.name + "##static-mesh-widget").c_str());
            RenderMaterial(other::fmtstr("{} material" , tag.name) , mesh.material);
            ImGui::Separator();
            ImGui::PopID();
          });
          ImGui::Separator();

          ImGui::Text(" - Light Controls =====");
          uint32_t i = 0;
          edited = false;
          reg.view<other::LightSource , other::Transform>().each([&](other::LightSource& light , other::Transform& transform) {
            switch (light.type) {
              case other::POINT_LIGHT_SRC:
                edited = edited || RenderPointLight(other::fmtstr("point light [{}]" , i++) , light.pointlight);
              break;
              case other::DIRECTION_LIGHT_SRC:
                edited = edited || RenderDirectionLight(other::fmtstr("direction light [{}]" , i++) , light.direction_light);
              break;
              default:
              break;
            }
            ++i;
          });

          if (edited) {
            scene->RebuildEnvironment();
          }

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

    mock_engine.UnloadApp();
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
  }
  
  return 1;
} 
