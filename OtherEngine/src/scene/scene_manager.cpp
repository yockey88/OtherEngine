/**
 * \file scene/scene_manager.cpp
 **/
#include "scene/scene_manager.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"

namespace other {

  void SceneManager::LoadScene(const std::string& scenepath) {
    UUID id = FNV(scenepath);
    if (loaded_scenes.find(id) != loaded_scenes.end()) {
      OE_WARN("Scene {} already loaded" , scenepath);
      return;
    }

    auto& scene_md = loaded_scenes[id] = {
      .path = scenepath ,
      .scene = Ref<Scene>::Create(scenepath)
    };

    scene_paths.push_back(scenepath);

    active_scene = &scene_md;
  }

  Ref<Scene>& SceneManager::ActiveScene() { 
    return active_scene->scene; 
  }
      
  const std::vector<std::string>& SceneManager::ScenePaths() const {
    return scene_paths;
  }

} // namespace other
