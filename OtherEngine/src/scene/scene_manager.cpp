/**
 * \file scene/scene_manager.cpp
 **/
#include "scene/scene_manager.hpp"

#include <fstream>

#include "core/defines.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "event/scene_events.hpp"
#include "input/mouse.hpp"

#include "ecs/entity.hpp"
#include "scene/scene_serializer.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/renderer.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  bool SceneManager::LoadScene(const Path& scenepath) {
    OE_DEBUG("Loading scene {}", scenepath);

    UUID id = FNV(scenepath.string());
    SceneSerializer serializer;
    {
      auto loaded_scene = serializer.Deserialize(scenepath.string());
      if (loaded_scene.scene == nullptr) {
        OE_ERROR("Failed to deserialize scene : {}", scenepath.string());
        return false;
      }

      loaded_scenes[id] = SceneMetadata{
        .name = loaded_scene.name,
        .path = scenepath,
        .scene_table = loaded_scene.scene_table,
        .scene = Ref<Scene>::Clone(loaded_scene.scene),
      };
    }

    scene_paths.push_back(scenepath.string());

    EventQueue::PushEvent<SceneLoad>({ id.Get() });

    return true;
  }

  void SceneManager::SetAsActive(const Path& path) {
    OE_DEBUG("Attempting to set scene {} to active", path);

    UUID id = FNV(path.string());
    auto find_scene = loaded_scenes.find(id);
    if (find_scene == loaded_scenes.end()) {
      OE_ERROR("Scene : {} does not exist! Cant not set as active", path);
      return;
    } else {
      OE_DEBUG("Setting {} as active scene", path);
    }

    active_scene = &loaded_scenes[id];

    ScriptEngine::SetSceneContext(active_scene->scene);
    Renderer::SetSceneContext(active_scene->scene);

    active_scene->scene->Initialize();
    auto primary_cam = active_scene->scene->GetPrimaryCamera();
    if (primary_cam != nullptr) {
      DefaultUpdateCamera(primary_cam);
    }
  }

  void SceneManager::StartScene() {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->Start();
    if (active_scene->scene->GetPrimaryCamera() != nullptr) {
      Mouse::LockCursor();
    }

    EventQueue::PushEvent<SceneStart>({ active_scene->scene->SceneHandle().Get() });
  }

  bool SceneManager::IsPlaying() const {
    if (!HasActiveScene()) {
      return false;
    }

    return active_scene->scene->IsRunning();
  }

  Ref<Scene> SceneManager::GetScene(UUID id) const {
    if (auto scn = loaded_scenes.find(id); scn != loaded_scenes.end()) {
      return Ref<Scene>::Clone(scn->second.scene);
    }

    return nullptr;
  }

  /// TODO: create state system so we don't have to reload the scene each time we stop it to reset
  ///         it to how it was.
  ///       this should also be the same system to handle undoing changes and stuff like that
  void SceneManager::StopScene() {
    if (active_scene == nullptr) {
      return;
    }

    OE_ASSERT(active_scene->scene != nullptr, "Active Scene has null scene reference!");

    if (!active_scene->scene->IsRunning()) {
      return;
    }

    active_scene->scene->Stop();

    EventQueue::PushEvent<SceneStop>({ active_scene->scene->SceneHandle().Get() });

#if 0  /// how to dynamically serialize scenes to only remember whats needed for undo/redo and
       /// also things that result from only manual changes and not scene update (to preserve 'initial' scene state)
    active_scene->scene->Shutdown();

    Path path = active_scene->path;
    UUID id = FNV(path.string());

    SceneSerializer serializer;
    {
      auto loaded_scene = serializer.Deserialize(path.string());
      if (loaded_scene.scene == nullptr) {
        OE_ERROR("Failed to reset scene while stopping : {}" , path.string());
        return;
      }

      loaded_scenes[id].scene = Ref<Scene>::Clone(loaded_scene.scene);
    }
    active_scene->scene->Initialize();
    SetAsActive(path);
#endif
  }

  bool SceneManager::HasScene(const Path& path) {
    return loaded_scenes.find(FNV(path.string())) != loaded_scenes.end();
  }

  bool SceneManager::HasActiveScene() const {
    return active_scene != nullptr && active_scene->scene != nullptr;
  }

  SceneMetadata* SceneManager::ActiveScene() const {
    if (!HasActiveScene()) {
      return nullptr;
    }
    return active_scene;
  }

  void SceneManager::SaveActiveScene() {
    if (!HasActiveScene()) {
      return;
    }

    Path active_path = active_scene->path;

    std::string scene_name = active_scene->name;
    Ref<Scene> scene = active_scene->scene;

    SceneSerializer serializer;
    std::stringstream ss;
    serializer.Serialize(scene_name, ss, scene);

    if (ss.str().size() == 0) {
      OE_WARN("Failed to serialize scene!");
    } else {
      std::ofstream scn_file(active_path);
      if (!scn_file.is_open()) {
        OE_ERROR("Failed to open scene file for scene {}", scene_name);
      } else {
        scn_file << ss.str();
      }
    }
  }

  void SceneManager::UnloadActive() {
    if (!HasActiveScene()) {
      return;
    }

    if (IsPlaying()) {
      StopScene();
    }

    active_scene->scene->Shutdown();

    ScriptEngine::SetSceneContext(nullptr);
    Renderer::SetSceneContext(nullptr);

    EventQueue::PushEvent<SceneUnload>({ active_scene->scene->SceneHandle().Get() });
    active_scene = nullptr;
  }

  StateCapture SceneManager::CaptureScene() {
    if (!HasActiveScene()) {
      return {};
    }

    return SaveStack::RecordState(ActiveScene()->scene);
  }

  void SceneManager::LoadCapture(StateCapture& capture) {
    if (!HasActiveScene()) {
      return;
    }

    bool scene_playing = false;
    if (active_scene->scene->IsRunning()) {
      active_scene->scene->Stop();
      scene_playing = true;
    }

    SaveStack::RestoreState(ActiveScene()->scene, capture);

    if (scene_playing) {
      active_scene->scene->Start(AppState::mode);
    }
  }

  void SceneManager::ClearScenes() {
    if (HasActiveScene()) {
      if (active_scene->scene->IsRunning()) {
        active_scene->scene->Stop();
      }
      active_scene->scene->Shutdown();
      active_scene = nullptr;
    }

    loaded_scenes.clear();
  }

  void SceneManager::LoadRenderer(Ref<SceneRenderer> renderer) {
    if (renderer == nullptr) {
      renderer = Renderer::DefaultSceneRenderer();
    }
    OE_ASSERT(renderer != nullptr, "Renderer is null!");

    scene_renderer = renderer;
  }

  const std::vector<std::string>& SceneManager::ScenePaths() const {
    return scene_paths;
  }

  const std::map<UUID, SceneMetadata>& SceneManager::GetScenes() const {
    return loaded_scenes;
  }

  void SceneManager::EarlyUpdateScene(float dt) {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->EarlyUpdate(dt);
  }

  void SceneManager::UpdateScene(float dt) {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->Update(dt);
  }

  void SceneManager::LateUpdateScene(float dt) {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->LateUpdate(dt);
  }

  bool SceneManager::RenderScene() {
    if (!HasActiveScene()) {
      return true;
    }

    OE_ASSERT(scene_renderer != nullptr, "Scene Renderer is null!");
    auto primary_cam = active_scene->scene->GetPrimaryCamera();
    if (primary_cam != nullptr) {
      scene_renderer->SubmitCamera(primary_cam);
    }

    active_scene->scene->Render(scene_renderer);
    bool render_success = scene_renderer->EndScene();

    //// debug rendering ?????
    //  debug rendering
    // scene_layer->bvh->RenderBounds("Debug", rendering_layer->renderer);
    // scene_layer->bvh->RenderEntityBounds("Debug", rendering_layer->renderer);
    // for (auto& [id, e] : scene->SceneEntities()) {
    //   OE_ASSERT(e != nullptr, "Entity is null");
    //   e->visited = false;
    // }
    ///

    /// present main frame and check whether to render to window or not
    /// TODO: customize which frame is considered the 'main' frame
    const auto& frames = scene_renderer->GetRender();
    auto itr = frames.find(FNV("Geometry"));
    if (render_success && itr != frames.end()) {
      const auto& vp = itr->second;
      Renderer::DrawFramebufferToWindow(vp);
    } else {
      return false;
    }

    return true;
  }

  void SceneManager::RenderSceneUI() {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->RenderUI();
  }

}  // namespace other
