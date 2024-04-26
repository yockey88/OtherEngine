/**
 * \file scene/scene_manager.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_MANAGER_HPP
#define OTHER_ENGINE_SCENE_MANAGER_HPP

#include <map>
#include <filesystem>

#include "core/uuid.hpp"
#include "core/ref.hpp"
#include "scene/scene.hpp"

namespace other {

  struct SceneMetadata {
    Path path;
    Ref<Scene> scene;
  };

  class SceneManager {
    public:
      void LoadScene(const std::string& scenepath);
      void SetAsActive(const std::string& name);

      Ref<Scene>& ActiveScene();

      const std::vector<std::string>& ScenePaths() const; 

    private:
      SceneMetadata* active_scene = nullptr;

      std::vector<std::string> scene_paths;
      std::map<UUID , SceneMetadata> loaded_scenes;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_MANAGER_HPP
