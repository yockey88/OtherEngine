/**
 * \file sandbox/main.cpp
 **/
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "other_engine.hpp"
#include "rendering_layer.hpp"
#include "scene_layer.hpp"

using namespace other;

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

    Ref<RenderingLayer> rendering_layer = NewRef<RenderingLayer>(&AppState::AppHandle(), "Rendering-Layer");
    Ref<SceneLayer> scene_layer = NewRef<SceneLayer>(&AppState::AppHandle(), "Scene-Layer");

    AppState::PushLayer(rendering_layer);
    AppState::PushLayer(scene_layer);
  }
};

OTHER_ENTRY_POINT(SandboxApp);

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