/**
 * \file sandbox/main.cpp
 **/
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "application/app.hpp"
#include "core/defines.hpp"
#include "core/engine.hpp"
#include "core/errors.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/time.hpp"

#include "event/event_queue.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "ecs/components/light_source.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/entity.hpp"
#include "scene/bvh.hpp"
#include "scene/environment.hpp"
#include "scene/scene.hpp"
#include "scene/scene_serializer.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/direction_light.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/geometry_pass.hpp"
#include "rendering/material.hpp"
#include "rendering/outline_pass.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/point_light.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/shader.hpp"
#include "rendering/ui/ui.hpp"
#include "rendering/ui/ui_helpers.hpp"
#include "rendering/ui/ui_widgets.hpp"
#include "rendering/uniform.hpp"
#include "scripting/script_engine.hpp"

#include "sandbox_ui.hpp"

using namespace other;

// clang-format off
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
// clang-format on

int main(int argc, char* argv[]) {
  try {
    const Path sandbox_dir = "C:/Yock/code/OtherEngine/tests/sandbox";
    const Path config_path = sandbox_dir / "sandbox.other";

    CmdLine cmd_line(argc, argv);
    cmd_line.SetFlag("--project", {config_path.string()});

    /// for test reasons
    Engine mock_engine(cmd_line);
    Logger::Open(mock_engine.config);
    Logger::Instance()->RegisterThread("Sandbox Thread");

    mock_engine.LoadApp();
    OE_DEBUG("Sandbox Launched");

    ScriptEngine::LoadProjectModules();

    const Path engine_core_dir = Filesystem::GetEngineCoreDir();
    const Path assets_dir = engine_core_dir / "OtherEngine" / "assets";

    const Path shader_dir = assets_dir / "shaders";
    const Path default_path = shader_dir / "default.oshader";
    const Path normals_path = shader_dir / "normals.oshader";
    const Path fbshader_path = shader_dir / "fbshader.oshader";
    const Path deferred_shader_path = shader_dir / "deferred_shading.oshader";
    const Path add_fog_shader_path = shader_dir / "fog.oshader";
    const Path red_path = shader_dir / "red.oshader";
    const Path outline_path = shader_dir / "outline.oshader";
    const Path pure_geometry_path = shader_dir / "pure_geometry.oshader";

    const Path texture_dir = assets_dir / "textures";
    const Path editor_texture_dir = texture_dir / "editor";
    const Path editor_folder_path = editor_texture_dir / "folder.png";

    const Path scene_dir = assets_dir / "scenes";

    const Path scenepath = sandbox_dir / "test_scene.yscn";
    OE_ASSERT(Filesystem::PathExists(scenepath), "Scene file does not exist : {}", scenepath.string());

    const Path bin_dir = engine_core_dir / "bin";
    const Path debug_bin_dir = bin_dir / "Debug";

    SDL_SetRelativeMouseMode(SDL_TRUE);

    {
      auto win_size = Renderer::WindowSize();
      Ref<CameraBase> camera = NewRef<PerspectiveCamera>(glm::ivec2{win_size.x, win_size.y});
      camera->SetPosition({0.f, 0.f, 3.f});

      Ref<Shader> fbshader = BuildShader(fbshader_path);

      Ref<VertexArray> fb_mesh = NewRef<VertexArray>(fb_verts, fb_indices, fb_layout);
      Ref<Framebuffer> framebuffer = NewRef<Framebuffer>(FramebufferSpec{
        .size = win_size,
      });

      uint32_t camera_binding_pnt = 0;
      std::vector<Uniform> cam_unis = {
        {"projection", ValueType::MAT4},
        {"view", ValueType::MAT4},
        {"viewpoint", ValueType::VEC4},
      };

      uint32_t model_binding_pnt = 1;
      std::vector<Uniform> model_unis = {
        {"models", ValueType::MAT4, 100},
      };

      uint32_t material_binding_pnt = 2;
      std::vector<Uniform> material_unis = {
        {"materials", ValueType::USER_TYPE, 100, sizeof(Material)},
      };

      uint32_t light_binding_pnt = 3;
      std::vector<Uniform> light_unis = {
        {"num_lights", ValueType::VEC4},
        {"point_lights", ValueType::USER_TYPE, 100, sizeof(PointLight)},
        {"direction_lights", ValueType::USER_TYPE, 100, sizeof(DirectionLight)},
      };

      Layout default_layout = {
        {ValueType::VEC3, "position"},
        {ValueType::VEC3, "normal"},
        {ValueType::VEC3, "tangent"},
        {ValueType::VEC3, "binormal"},
        {ValueType::VEC2, "uvs"}};

      std::vector<Uniform> outline_unis = {
        {"outline_color", VEC3},
      };
      std::vector<Uniform> geometry_unis = {};
      Ref<Shader> outline_shader = BuildShader(outline_path);
      Ref<Shader> geometry_shader = BuildShader(default_path);

      RenderPassSpec normal_pass_spec{
        .name = "Draw Normals",
        .tag_col = {0.f, 1.f, 0.f, 1.f},
        .uniforms = {
          {"magnitude", ValueType::FLOAT},
        },
        .shader = BuildShader(normals_path),
      };
      Ref<RenderPass> normal_pass = NewRef<RenderPass>(normal_pass_spec);
      normal_pass->SetInput("magnitude", 0.2f);

      RenderPassSpec pure_geom_pass_spec{
        .name = "Pure Geometry",
        .tag_col = {0.f, 0.f, 1.f, 1.f},
        .uniforms = {},
        .shader = BuildShader(pure_geometry_path),
      };
      Ref<RenderPass> pure_geom_pass = NewRef<RenderPass>(pure_geom_pass_spec);

      Ref<RenderPass> geom_pass = NewRef<GeometryPass>(geometry_unis, geometry_shader);
      Ref<RenderPass> outline_pass = NewRef<OutlinePass>(outline_unis, outline_shader);

      SceneRenderSpec render_spec{
        .camera_uniforms = NewRef<UniformBuffer>("Camera", cam_unis, camera_binding_pnt),
        .light_uniforms = NewRef<UniformBuffer>("Lights", light_unis, light_binding_pnt, SHADER_STORAGE),
        .pipelines = {
          {
            .topology = DrawMode::TRIANGLES,
            .framebuffer_spec = {
              .depth_func = LESS_EQUAL,
              .clear_color = {0.1f, 0.1f, 0.1f, 1.f},
              .size = Renderer::WindowSize(),
            },
            .vertex_layout = default_layout,
            .model_uniforms = model_unis,
            .model_binding_point = model_binding_pnt,
            .material_uniforms = material_unis,
            .material_binding_point = material_binding_pnt,
            .debug_name = "Geometry",
          },
          {
            .topology = DrawMode::TRIANGLES,
            .framebuffer_spec = {
              .depth_func = LESS_EQUAL,
              .clear_color = {0.1f, 0.1f, 0.1f, 1.f},
              .size = Renderer::WindowSize(),
            },
            .vertex_layout = default_layout,
            .model_uniforms = model_unis,
            .model_binding_point = model_binding_pnt,
            .material_uniforms = material_unis,
            .material_binding_point = material_binding_pnt,
            .debug_name = "Debug",
          },
        },
        .passes = {
          geom_pass,
          normal_pass,
          pure_geom_pass,
        },
        .pipeline_to_pass_map = {
          {FNV("Geometry"), {FNV(geom_pass->Name())}}, {FNV("Debug"), {FNV(pure_geom_pass->Name())}},  // , FNV(normal_pass->Name())
        },
      };
      Ref<SceneRenderer> renderer = NewRef<SceneRenderer>(render_spec);

      bool running = true;
      bool camera_lock = true;

      if (camera_lock) {
        Mouse::FreeCursor();
      } else {
        Mouse::LockCursor();
      }

      Material cube_material1 = {
        .color = {1.0f, 0.5f, 0.31f, 1.f},
        .shininess = 32.f,
      };

      Material cube_material2 = {
        .color = {0.1f, 0.5f, 0.31f, 1.f},
        .shininess = 32.f,
      };

      const glm::vec3 light_scale = glm::vec3(0.2f, 0.2f, 0.2f);

      glm::mat4 light_model1 = glm::mat4(1.f);
      glm::vec3 light_pos1 = glm::vec3(1.2f, 1.f, 2.f);
      light_model1 = glm::translate(light_model1, light_pos1);
      light_model1 = glm::scale(light_model1, light_scale);

      glm::mat4 light_model2 = glm::mat4(1.f);
      glm::vec3 light_pos2 = glm::vec3(-1.2f, 1.f, 2.f);
      light_model2 = glm::translate(light_model2, light_pos2);
      light_model2 = glm::scale(light_model2, light_scale);

      glm::vec3 outline_color{1.f, 0.f, 0.f};

      SceneSerializer serializer;
      Ref<Scene> scene = nullptr;
      {
        auto loaded_scene = serializer.Deserialize(scenepath.string());
        if (loaded_scene.scene == nullptr) {
          OE_ERROR("Failed to deserialize scene : {}", scenepath.string());
          throw std::runtime_error("Failed to deserialize scene!");
        }
        scene = loaded_scene.scene;
      }

      Ref<BvhTree> bvh = NewRef<BvhTree>(glm::vec3{0.f, 0.f, 0.f});

      bvh->Subdivide({10.f, 10.f, 10.f}, 3);
      bvh->AddScene(scene, glm::zero<glm::vec3>());

      auto* cube = scene->GetEntity("cube");
      auto* floor = scene->GetEntity("floor");
      auto* sun = scene->GetEntity("sun");

      auto& cube_mesh = cube->GetComponent<StaticMesh>();
      cube_mesh.material = cube_material1;

      auto& floor_mesh = floor->GetComponent<StaticMesh>();
      floor_mesh.material = cube_material2;

      auto& sun_l = sun->GetComponent<LightSource>();
      sun_l.direction_light = {
        .direction = {0.f, -1.f, 0.f, 1.f},
        .color = {0.22f, 0.22f, 0.11f, 1.f},
      };

      ScriptEngine::SetSceneContext(scene);
      Renderer::SetSceneContext(scene);

      scene->Initialize();
      scene->Start();
      DefaultUpdateCamera(camera);

      OE_DEBUG("Lights [{} , {}]", scene->GetEnvironment()->direction_lights.size(),
               scene->GetEnvironment()->point_lights.size());

      OE_INFO("Running");

      bool render_to_window = true;
      // renderer->ToggleWireframe();

      time::DeltaTime dt;
      dt.Start();
#if 0
      running = false;
#endif
      while (running) {
        float delta = dt.Get();
        IO::Update();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
          switch (event.type) {
            case SDL_QUIT:
              running = false;
              break;
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
                    Mouse::FreeCursor();
                  } else {
                    Mouse::LockCursor();
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

        EventQueue::Clear();

        if (!camera_lock) {
          DefaultUpdateCamera(camera);
        }

        scene->EarlyUpdate(delta);
        scene->Update(delta);
        scene->LateUpdate(delta);

        bvh->Update();

        Renderer::GetWindow()->Clear();
        // renderer->SetRenderMode(Render)

        auto cam = scene->GetPrimaryCamera();
        if (cam == nullptr) {
          renderer->SubmitCamera(/* editor camera */ camera);
        }
        scene->Render(renderer);

        /// debug rendering
        bvh->RenderBounds("Debug", renderer);
        bvh->RenderEntityBounds("Debug", renderer);
        for (auto& [id, e] : scene->SceneEntities()) {
          OE_ASSERT(e != nullptr, "Entity is null");
          e->visited = false;
        }
        ///

        bool success = renderer->EndScene();

        const auto& frames = renderer->GetRender();

        auto itr = frames.find(FNV("Geometry"));
        if (itr != frames.end()) {
          const auto& vp = itr->second;
          if (render_to_window) {
            Renderer::DrawFramebufferToWindow(vp);
          } else {
          }
        }

#define UI_ENABLED 1
#if UI_ENABLED
        /// lambda to get fps from delta
        auto fps = [](float dt) -> float {
          return (1.f / dt) * 1000.f;
        };

        UI::BeginFrame();
        const ImVec2 win_size = {(float)Renderer::WindowSize().x, (float)Renderer::WindowSize().y};
        if (ImGui::Begin("Stats")) {
          ImGui::Text("FPS : %.2f", fps(delta));
        }
        ImGui::End();

        if (ImGui::Begin("Frames")) {
          if (!success) {
            ScopedColor red(ImGuiCol_Text, ImVec4(1.f, 0.f, 0.f, 1.f));
            ImGui::Text("Failed to render frame");
          } else {
            ImGui::Text("Frames %llu", frames.size());
            if (auto frame = frames.find(FNV("Debug")); frame != frames.end()) {
              RenderItem(frame->second->texture, "Debug", ImVec2(win_size.x, win_size.y));
            }
          }
        }
        ImGui::End();

        if (ImGui::Begin("Render Settings")) {
          bool edited = false;
          ui::Underline();

          ImGui::Text("Debug Controls");
          ImGui::Separator();
          ui::widgets::DrawVec3Control("outline color", outline_color, edited, 0.f, 100.f, ui::VectorAxis::ZERO,
                                       {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, 0.1f);
          ImGui::Separator();

          ImGui::Text("===== Scene Controls =====");
          auto& reg = scene->Registry();

          ImGui::Text(" - Transforms =====");
          reg.view<Tag, Transform>().each([&](Tag& tag, Transform& transform) {
            ImGui::PushID((tag.name + "##transform-widget").c_str());
            if (ui::widgets::DrawVec3Control(fmtstr("{} position", tag.name),
                                             transform.position, edited, 0.f, 100.f, ui::VectorAxis::ZERO,
                                             {-100.f, -100.f, -100.f}, {100.f, 100.f, 100.f}, 0.5f)) {}
            ImGui::Separator();
            ImGui::PopID();
          });

          ImGui::Text(" - Materials =====");

          reg.view<Tag, StaticMesh>().each([&](Tag& tag, StaticMesh& mesh) {
            ImGui::PushID((tag.name + "##static-mesh-widget").c_str());
            RenderMaterial(fmtstr("{} material", tag.name), mesh.material);
            ImGui::Separator();
            ImGui::PopID();
          });
          ImGui::Separator();

          ImGui::Text(" - Light Controls =====");
          uint32_t i = 0;
          edited = false;
          reg.view<LightSource, Transform>().each([&](LightSource& light, Transform& transform) {
            switch (light.type) {
              case POINT_LIGHT_SRC:
                edited = edited || RenderPointLight(fmtstr("point light [{}]", i++), light.pointlight);
                break;
              case DIRECTION_LIGHT_SRC:
                edited = edited || RenderDirectionLight(fmtstr("direction light [{}]", i++), light.direction_light);
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
        UI::EndFrame();
#endif  // !UI_ENABLED

        Renderer::GetWindow()->SwapBuffers();
      }

      scene->Stop();
      scene->Shutdown();
    }

    mock_engine.UnloadApp();
    OE_INFO("Succesful exit");
    Logger::Shutdown();

    return 0;
  } catch (const IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
  } catch (const ShaderException& e) {
    std::cout << "caught shader error : " << e.what() << "\n";
  } catch (const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
  } catch (...) {
    std::cout << "unknown error" << "\n";
  }

  return 1;
}
