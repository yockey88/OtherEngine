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

#include "core/defines.hpp"
#include "core/engine.hpp"
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "core/time.hpp"

#include "application/app.hpp"
#include "event/event_queue.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/renderer.hpp"
#include "rendering/scene_renderer.hpp"
#include "scripting/script_engine.hpp"

#define UI_ENABLED 1
#if UI_ENABLED
#include "rendering/ui/ui.hpp"
#include "rendering/ui/ui_helpers.hpp"

#include "sandbox_ui.hpp"
#endif

#include "control_layer.hpp"
#include "rendering_layer.hpp"
#include "scene_layer.hpp"

using namespace other;

int main() {
  try {
    const std::vector<Arg> sandbox_cmd_line = {
      Arg("--project", { "C:/Yock/code/OtherEngine/tests/sandbox/sandbox.other" })
    };

    CmdLine cmd_line(sandbox_cmd_line);

    /// for test reasons
    Engine mock_engine(cmd_line);
    Logger::Open(mock_engine.config);
    Logger::Instance()->RegisterThread("Sandbox Thread");

    mock_engine.LoadApp();
    OE_DEBUG("Sandbox Launched");

    {
      Ref<ControlLayer> control_layer = NewRef<ControlLayer>(&AppState::AppHandle(), "Control-Layer");
      Ref<RenderingLayer> rendering_layer = NewRef<RenderingLayer>(&AppState::AppHandle(), "Rendering-Layer");
      Ref<SceneLayer> scene_layer = NewRef<SceneLayer>(&AppState::AppHandle(), "Scene-Layer");
      AppState::PushLayer(control_layer);
      AppState::PushLayer(rendering_layer);
      AppState::PushLayer(scene_layer);
      EventQueue::Poll();

      Ref<Scene> scene = scene_layer->scene;
      ScriptRef<LuaObject> sandbox_ui = ScriptEngine::GetScriptObject("SandboxUI", "", "sandbox_ui");
      sandbox_ui->Initialize();

      DefaultUpdateCamera(rendering_layer->camera);

      bool render_to_window = true;

      mock_engine.Start();
      OE_INFO("Running");
      while (control_layer->running) {
        /// engine tick, will send events to event queue and dispatch them to listeners
        ///   and will trigger an application tick every ??? seconds
        mock_engine.Tick();

        if (!control_layer->camera_lock) {
          DefaultUpdateCamera(rendering_layer->camera);
        }

        AppState::RunEarlyUpdate();
        AppState::RunUpdate();
        AppState::RunLateUpdate();

        Renderer::GetWindow()->Clear();

        auto cam = scene->GetPrimaryCamera();
        if (cam == nullptr) {
          rendering_layer->renderer->SubmitCamera(/* editor camera */ rendering_layer->camera);
        }

        scene->Render(rendering_layer->renderer);

        // / debug rendering
        scene_layer->bvh->RenderBounds("Debug", rendering_layer->renderer);
        scene_layer->bvh->RenderEntityBounds("Debug", rendering_layer->renderer);
        for (auto& [id, e] : scene->SceneEntities()) {
          OE_ASSERT(e != nullptr, "Entity is null");
          e->visited = false;
        }
        ///

        bool success = rendering_layer->renderer->EndScene();

        const auto& frames = rendering_layer->renderer->GetRender();
        auto itr = frames.find(FNV("Geometry"));
        if (itr != frames.end()) {
          const auto& vp = itr->second;
          if (render_to_window) {
            Renderer::DrawFramebufferToWindow(vp);
          } else {
          }
        }

#if UI_ENABLED
        /// lambda to get fps from delta
        auto fps = [](float dt) -> float {
          return (1.f / dt) * 1000.f;
        };

        UI::BeginFrame();

        sandbox_ui->RenderUI();
        const ImVec2 win_size = { (float)Renderer::WindowSize().x, (float)Renderer::WindowSize().y };

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
        UI::EndFrame();
#endif  // !UI_ENABLED

        Renderer::GetWindow()->SwapBuffers();
      }
      mock_engine.Stop();

      // scene_layer->scene->Stop();
      // scene_layer->scene->Shutdown();

      sandbox_ui->Shutdown();
      sandbox_ui = nullptr;
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

/// trace mouse cursor ray
/**
int32_t mouse_x , mouse_y;
SDL_GetMouseState(&mouse_x , &mouse_y);

glm::vec3 mouse_screen_pos{
static_cast<float>(mouse_x) / renderer->window->size.x ,
static_cast<float>(mouse_y) / renderer->window->size.y ,
1.f
};

glm::vec4 homo_clip_pos{
mouse_screen_pos.x , mouse_screen_pos.y ,
-1.f , 1.f
};

glm::vec4 cam_pos = glm::inverse(camera.Projection()) * homo_clip_pos;
glm::vec4 eye_pos{
cam_pos.x , cam_pos.y ,
-1.f , 1.f
};

glm::vec4 intermediate_world_pos = glm::inverse(camera.View()) * eye_pos;
glm::vec3 mouse_ray_direction = glm::normalize(glm::vec3{
intermediate_world_pos.x ,
intermediate_world_pos.y ,
intermediate_world_pos.z
});

scene->TraceRay(Ray(origin = camera->position , direction = mouse_ray_direction))
 **/

// engine_core_dir = Filesystem::GetEngineCoreDir();
// assets_dir = engine_core_dir / "OtherEngine" / "assets";

// shader_dir = assets_dir / "shaders";
// default_path = shader_dir / "default.oshader";
// normals_path = shader_dir / "normals.oshader";
// fbshader_path = shader_dir / "fbshader.oshader";
// deferred_shader_path = shader_dir / "deferred_shading.oshader";
// add_fog_shader_path = shader_dir / "fog.oshader";
// red_path = shader_dir / "red.oshader";
// outline_path = shader_dir / "outline.oshader";
// pure_geometry_path = shader_dir / "pure_geometry.oshader";

// texture_dir = assets_dir / "textures";
// editor_texture_dir = texture_dir / "editor";
// editor_folder_path = editor_texture_dir / "folder.png";

// scene_dir = assets_dir / "scenes";

// scenepath = Path("C:/Yock/code/OtherEngine/tests/sandbox") / "test_scene.yscn";
// OE_ASSERT(Filesystem::PathExists(scenepath), "Scene file does not exist : {}", scenepath.string());

// bin_dir = engine_core_dir / "bin";
// debug_bin_dir = bin_dir / "Debug";