/**
 * \file scene/scene_manager.cpp
 **/
#include "scene/scene_manager.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "asset/asset_database.hpp"
#include "asset/asset_manager.hpp"
#include "event/event_queue.hpp"
#include "event/scene_events.hpp"
#include "input/mouse.hpp"

#include "scene/bvh.hpp"
#include "scene/scene_serializer.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/renderer.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  void SceneManager::Unload() {
    if (HasActiveScene()) {
      UnloadActive();
    }

    for (auto& [id, scene] : loaded_scenes) {
      scene.scene->Shutdown();
    }

    loaded_scenes.clear();
    scene_paths.clear();
  }

  bool SceneManager::LoadScene(const Ref<FileHandle>& scene_file) {
    OE_ASSERT(scene_file != nullptr, "Attempting to load null scene file");
    OE_INFO("Loading scene {}", Path(*scene_file));
    if (!scene_file->Exists()) {
      OE_ERROR("Scene file does not exist : {}", Path(*scene_file));
      return false;
    }

    if (scene_file->GetAssetType() != AssetType::SCENE) {
      OE_ERROR("File is not a .yscn file : {}", Path(*scene_file));
      return false;
    }

    UUID file_handle = scene_file->handle;
    AssetKey key = {
      .file_handle = file_handle,
      .type = AssetType::SCENE,
    };
    OE_DEBUG("Scene = {}", key);

    /// will force load if not already loaded
    Ref<Scene> scene = AssetManager::GetAsset<Scene>(key);
    OE_ASSERT(scene != nullptr, "Failed to get scene asset : {}", Path(*scene_file));

    SceneMetadata* scene_md = GetSceneMetadata(scene->scene_handle);
    OE_ASSERT(scene_md != nullptr, "Failed to get scene metadata for scene : {}", scene->scene_handle);

    OE_TRACE(" > loaded scene : {}", scene_md->name);
    EventQueue::PushEvent<SceneLoad>({ scene->scene_handle.Get() });
    return true;
  }

  void SceneManager::AddScene(const DeserializedScene& scene) {
    OE_ASSERT(scene.scene != nullptr, "Attempting to add null scene");

    UUID id = scene.scene->scene_handle;
    if (loaded_scenes.find(id) != loaded_scenes.end()) {
      OE_WARN("Scene already loaded : {}", scene.name);
      return;
    }

    loaded_scenes[id] = SceneMetadata{
      .name = scene.name,
      .path = scene.path,
      .scene_table = scene.scene_table,
      .scene = Ref<Scene>::Clone(scene.scene),
      .corrupted = false
    };

    loaded_scenes[id].scene->Initialize();
    scene_paths.push_back(scene.path.string());
  }

  void SceneManager::SetAsActive(const Ref<FileHandle>& scenefile) {
    OE_ASSERT(scenefile != nullptr, "Attempting to set null scene as active");

    AssetKey key = {
      .file_handle = scenefile->handle,
      .type = AssetType::SCENE,
    };
    Ref<Scene> scene = AssetManager::GetAsset<Scene>(key);

    OE_DEBUG("Attempting to set scene {} to active [{}]", scene->scene_name, scene->scene_handle);

    auto find_scene = loaded_scenes.find(scene->scene_handle);
    if (find_scene == loaded_scenes.end()) {
      OE_ERROR("Failed to set scene {} as active : not found", scene->scene_name);
      return;
    }

    OE_ASSERT(find_scene->second.scene != nullptr, "Scene is null in loaded scenes");
    OE_DEBUG("Setting {} as active scene", scene->scene_name);
    active_scene = &find_scene->second;
    OE_ASSERT(active_scene != nullptr, "Failed to set active scene!");
    OE_ASSERT(active_scene->scene != nullptr, "Active scene has no scene!");

    active_scene->bvh = NewRef<BvhTree>(glm::vec3{ 0.f, 0.f, 0.f });
    active_scene->bvh->AddScene(active_scene->scene, glm::zero<glm::vec3>());

    ScriptEngine::SetSceneContext(active_scene->scene);
    Renderer::SetSceneContext(active_scene->scene);
    auto primary_cam = active_scene->scene->GetPrimaryCamera();
    if (primary_cam != nullptr) {
      DefaultUpdateCamera(primary_cam);
    }

    EventQueue::PushEvent<SceneActivate>({ active_scene->scene->SceneHandle().Get() });
  }

  void SceneManager::Activate(Ref<Scene>& scene) {
    if (scene == nullptr) {
      OE_ERROR("Attempting to activate null scene!");
      return;
    }

    UUID id = scene->scene_handle;
    if (HasActiveScene()) {
      UnloadActive();
    }

    auto itr = loaded_scenes.find(id);
    if (itr == loaded_scenes.end()) {
      loaded_scenes[id] = SceneMetadata{
        .scene = scene,
        .bvh = NewRef<BvhTree>(glm::vec3{ 0.f, 0.f, 0.f }),
        .corrupted = false
      };
      itr = loaded_scenes.find(id);
      itr->second.bvh->AddScene(scene, glm::zero<glm::vec3>());
    }

    SceneMetadata* active_scene = &itr->second;
    OE_ASSERT(active_scene != nullptr, "Failed to set active scene!");
    ScriptEngine::SetSceneContext(active_scene->scene);
    Renderer::SetSceneContext(active_scene->scene);

    auto primary_cam = active_scene->scene->GetPrimaryCamera();
    if (primary_cam != nullptr) {
      DefaultUpdateCamera(primary_cam);
    }

    EventQueue::PushEvent<SceneActivate>({ active_scene->scene->SceneHandle().Get() });
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

  SceneMetadata* SceneManager::GetSceneMetadata(UUID id) {
    if (auto scn = loaded_scenes.find(id); scn != loaded_scenes.end()) {
      return &scn->second;
    }

    return nullptr;
  }

  void SceneManager::RemoveScene(UUID id) {
    if (HasActiveScene() && active_scene->scene->SceneHandle() == id) {
      UnloadActive();
    }

    if (auto scn = loaded_scenes.find(id); scn != loaded_scenes.end()) {
      loaded_scenes[id].scene->Shutdown();
      loaded_scenes.erase(scn);
    }
  }

  void SceneManager::RemoveScene(const std::string_view name) {
    if (HasActiveScene() && active_scene->name == name) {
      UnloadActive();
    }

    for (auto& [id, scene] : loaded_scenes) {
      if (scene.name == name) {
        loaded_scenes[id].scene->Shutdown();
        loaded_scenes.erase(id);
        return;
      }
    }
  }

  Ref<SceneRenderer> SceneManager::GetRenderer() {
    if (scene_renderer == nullptr) {
      OE_DEBUG("No Scene Renderer set, using default renderer");
      scene_renderer = Renderer::DefaultSceneRenderer();
    }
    return scene_renderer;
  }

  /// TODO: create state-capture system so we don't have to reload the scene each time we stop it to reset
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

/// TODO: how to dynamically serialize scenes to only remember whats needed for undo/redo and
///         also things that result from only manual changes and not scene update (to preserve 'initial' scene state)
#if 0  
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

  bool SceneManager::HasScene(UUID id) {
    return loaded_scenes.find(id) != loaded_scenes.end();
  }

  bool SceneManager::HasScene(const std::string_view name) {
    return !(
      loaded_scenes.find(FNV(name)) == loaded_scenes.end() ||
      std::ranges::find_if(loaded_scenes, [&](const auto& pair) -> bool { return pair.second.name == name; }) == loaded_scenes.end()
    );
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

    // Path active_path = active_scene->path;

    // std::string scene_name = active_scene->name;
    // Ref<Scene> scene = active_scene->scene;

    // SceneSerializer serializer;
    // std::stringstream ss;
    // serializer.Serialize(scene_name, ss, scene);

    // if (ss.str().size() == 0) {
    //   OE_WARN("Failed to serialize scene!");
    // } else {
    //   std::ofstream scn_file(active_path);
    //   if (!scn_file.is_open()) {
    //     OE_ERROR("Failed to open scene file for scene {}", scene_name);
    //   } else {
    //     scn_file << ss.str();
    //   }
    // }
  }

  void SceneManager::UnloadActive() {
    if (!HasActiveScene()) {
      return;
    }

    if (IsPlaying()) {
      StopScene();
    }

    active_scene->corrupted = false;

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
    active_scene->bvh->Update();
  }

  bool SceneManager::RenderScene() {
    if (!HasActiveScene()) {
      return true;
    }

    OE_ASSERT(scene_renderer != nullptr, "Scene Renderer is null!");

    /// render scene
    active_scene->scene->Render(scene_renderer);

    /// let editor render more stuff on top
    if (AppState::mode != EngineMode::EDITOR) {
      return scene_renderer->FinalizeScene();
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
