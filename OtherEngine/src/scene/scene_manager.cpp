/**
 * \file scene/scene_manager.cpp
 **/
#include "scene/scene_manager.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"

namespace other {

  void SceneManager::LoadScene(const Path& scenepath) {
    UUID id = FNV(scenepath.string());
    if (loaded_scenes.find(id) != loaded_scenes.end()) {
      OE_WARN("Scene {} already loaded" , scenepath);
      return;
    }

    auto& scene_md = loaded_scenes[id] = {
      .path = scenepath ,
      .scene = Ref<Scene>::Create(scenepath)
    };

    scene_paths.push_back(scenepath.string());

    active_scene = &scene_md;
  }

  void SceneManager::SetAsActive(const Path& path) {
    UUID id = FNV(path.string());
    auto find_scene = loaded_scenes.find(id);
    if (find_scene == loaded_scenes.end()) {
      OE_WARN("Scene : {} does not exist! Cant not set as active" , path);
      return;
    }

    active_scene = &loaded_scenes[id];
  }

  bool SceneManager::HasScene(const Path& path) {
    return loaded_scenes.find(FNV(path.string())) != loaded_scenes.end();
  }

  Ref<Scene> SceneManager::ActiveScene() { 
    if (active_scene == nullptr) {
      return nullptr;
    }
    return Ref<Scene>::Clone(active_scene->scene);
  }

  void SceneManager::UnloadActive() {
    active_scene->scene->Stop();
    active_scene = nullptr;
  }
      
  const std::vector<std::string>& SceneManager::ScenePaths() const {
    return scene_paths;
  }

} // namespace other
