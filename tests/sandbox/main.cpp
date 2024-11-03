/**
 * \file sandbox/main.cpp
 **/
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "control_layer.hpp"
#include "engine/engine.hpp"
#include "rendering_layer.hpp"
#include "scene_layer.hpp"

using namespace other;

#include <iostream>

bool ProcessFileMod(const ModifyFileEvent& event) {
  Ref<FileHandle> file = Filesystem::GetFile(event.handle);
  OE_ASSERT(file != nullptr, "Failed to get file handle for event : {}", event.handle);

  if (file->GetAssetType() == AssetType::SCRIPTFILE) {
    /// reload script
    OE_INFO("Reloading script : {}", file->AbsolutePath());
  }

  return false;
}
class SandboxApp : public other::App {
 public:
  SandboxApp(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~SandboxApp() override {}

  virtual void OnAttach() override {
    OE_INFO("Sandbox App Attached");
    EventQueue::RegisterEventDispatcher<ModifyFileEvent>(
      "Sandbox-File-Listener",
      { &ProcessFileMod }
    );

    Ref<ControlLayer> control_layer = NewRef<ControlLayer>(&AppState::AppHandle(), "Control-Layer");
    Ref<RenderingLayer> rendering_layer = NewRef<RenderingLayer>(&AppState::AppHandle(), "Rendering-Layer");
    Ref<SceneLayer> scene_layer = NewRef<SceneLayer>(&AppState::AppHandle(), "Scene-Layer");
    AppState::PushLayer(control_layer);
    AppState::PushLayer(rendering_layer);
    AppState::PushLayer(scene_layer);
  }
};

namespace other {
  App* NewApp(const CmdLine& cmd_line, const ConfigTable& config);
}  // namespace other

int main() {
  try {
    const std::vector<Arg> sandbox_cmd_line = {
      Arg("--project", { "C:/Yock/code/OtherEngine/tests/sandbox/sandbox.other" })
    };

    CmdLine cmd_line(sandbox_cmd_line);

    /// for test reasons
    Engine mock_engine(cmd_line, "Sandbox Thread");
    OE_DEBUG("Sandbox Launched");
    {
      mock_engine.Start();
      OE_INFO("Running");
      do {
        mock_engine.Step();
      } while (!mock_engine.exit_code.has_value());
      mock_engine.Stop();
    }
    OE_INFO("Successful exit");
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

namespace other {
  App* NewApp(const CmdLine& cmd_line, const ConfigTable& config) {
    return new SandboxApp(cmd_line, config);
  }
}  // namespace other

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