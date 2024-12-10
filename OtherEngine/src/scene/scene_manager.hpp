/**
 * \file scene/scene_manager.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_MANAGER_HPP
#define OTHER_ENGINE_SCENE_MANAGER_HPP

#include <map>

#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "scene/bvh.hpp"
#include "scene/scene.hpp"
#include "scene/scene_serializer.hpp"

#include "rendering/scene_renderer.hpp"

#include "editor/saves.hpp"

namespace other {

  struct SceneMetadata {
    std::string name;
    Path path;
    ConfigTable scene_table;
    Ref<Scene> scene = nullptr;
    Ref<BvhTree> bvh = nullptr;
    bool corrupted = false;
  };

  class SceneManager {
   public:
    SceneManager() {}
    ~SceneManager() {}

    void Unload();

    bool LoadScene(const Ref<FileHandle>& scenepath);
    void AddScene(const DeserializedScene& scene);
    void SetAsActive(const Ref<FileHandle>& scenefile);

    void Activate(Ref<Scene>& scene);

    void StartScene();
    void StopScene();

    bool IsPlaying() const;

    Ref<Scene> GetScene(UUID id) const;
    SceneMetadata* GetSceneMetadata(UUID id);

    void RemoveScene(UUID id);
    void RemoveScene(const std::string_view name);

    Ref<SceneRenderer> GetRenderer();

    bool HasScene(UUID id);
    bool HasScene(const std::string_view name);
    bool HasActiveScene() const;

    SceneMetadata* ActiveScene() const;
    void SaveActiveScene();
    void UnloadActive();

    StateCapture CaptureScene();
    void LoadCapture(StateCapture& capture);

    void ClearScenes();

    void LoadRenderer(Ref<SceneRenderer> renderer);

    const std::vector<std::string>& ScenePaths() const;
    const std::map<UUID, SceneMetadata>& GetScenes() const;

    void EarlyUpdateScene(float dt);
    void UpdateScene(float dt);
    void LateUpdateScene(float dt);
    bool RenderScene();
    void RenderSceneUI();

   private:
    bool playing_scene = false;
    SceneMetadata* active_scene = nullptr;
    Ref<SceneRenderer> scene_renderer = nullptr;

    std::vector<std::string> scene_paths;
    std::map<UUID, SceneMetadata> loaded_scenes;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_MANAGER_HPP
