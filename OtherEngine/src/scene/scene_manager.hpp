/**
 * \file scene/scene_manager.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_MANAGER_HPP
#define OTHER_ENGINE_SCENE_MANAGER_HPP

#include <map>

#include "core/defines.hpp"
#include "core/config.hpp"
#include "core/uuid.hpp"
#include "core/ref.hpp"
#include "scene/scene.hpp"

namespace other {

  struct SceneMetadata {
    std::string name;
    Path path;
    ConfigTable scene_table;
    Ref<Scene> scene;
  };

  class SceneManager {
    public:
      SceneManager() {}
      ~SceneManager() {}

      bool LoadScene(const Path& scenepath);
      void SetAsActive(const Path& name);

      void StartScene();
      void StopScene();

      bool HasScene(const Path& path);
      bool HasActiveScene() const;

      SceneMetadata* ActiveScene() const;
      void SaveActiveScene();
      void UnloadActive();

      void ClearScenes();

      const std::vector<std::string>& ScenePaths() const; 
      const std::map<UUID , SceneMetadata>& GetScenes() const;

      void UpdateScene(float dt);
      void RenderScene();
      void RenderSceneUI();
      void LateUpdateScene(float dt);

    private:
      SceneMetadata* active_scene = nullptr;

      std::vector<std::string> scene_paths;
      std::map<UUID , SceneMetadata> loaded_scenes;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_MANAGER_HPP
