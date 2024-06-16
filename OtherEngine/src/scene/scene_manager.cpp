/**
 * \file scene/scene_manager.cpp
 **/
#include "scene/scene_manager.hpp"

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "ecs/entity.hpp"
#include "scene/scene_serializer.hpp"

namespace other {

  bool SceneManager::LoadScene(const Path& scenepath) {
    UUID id = FNV(scenepath.string());
    if (auto scn = loaded_scenes.find(id); scn != loaded_scenes.end()) {
      return true;
    }

    SceneSerializer serializer;
    {
      auto loaded_scene = serializer.Deserialize(scenepath.string());
      if (loaded_scene.scene == nullptr) {
        OE_ERROR("Failed to deserialize scene : {}" , scenepath.string());
        return false;
      }

      loaded_scenes[id] = SceneMetadata{
        .name = loaded_scene.name ,
        .path = scenepath,
        .scene_table = loaded_scene.scene_table,
        .scene = Ref<Scene>::Clone(loaded_scene.scene) ,
      };
    }

    scene_paths.push_back(scenepath.string());

    return true;
  }

  void SceneManager::SetAsActive(const Path& path) {
    UUID id = FNV(path.string());
    auto find_scene = loaded_scenes.find(id);
    if (find_scene == loaded_scenes.end()) {
      OE_WARN("Scene : {} does not exist! Cant not set as active" , path);
      return;
    } else {
      OE_DEBUG("Setting {} as active scene" , path);
    }

    active_scene = &loaded_scenes[id];
    active_scene->scene->Initialize();
  }
      
  void SceneManager::StartScene() {
    if (active_scene == nullptr) {
      return;
    } 

    active_scene->scene->Start();
  }
  
  void SceneManager::StopScene() {
    if (active_scene == nullptr) {
      return;
    } 

    if (!active_scene->scene->IsRunning()) {
      return;
    }

    active_scene->scene->Stop();
  }

  bool SceneManager::HasScene(const Path& path) {
    return loaded_scenes.find(FNV(path.string())) != loaded_scenes.end();
  }

  bool SceneManager::HasActiveScene() const {
    return active_scene != nullptr && active_scene->scene != nullptr;
  }
      
  void SceneManager::RefreshScenes() {
    for (auto& [id , scene] : loaded_scenes) {
      scene.scene->Refresh();
    } 
  }

  const SceneMetadata* SceneManager::ActiveScene() const { 
    if (!HasActiveScene()) {
      return nullptr;
    }
    return active_scene;
  }

  void SceneManager::UnloadActive() {
    if (!HasActiveScene()) {
      return;
    }

    if (active_scene->scene->IsRunning()) {
      active_scene->scene->Stop();
    }
    
    active_scene->scene->Shutdown();
    active_scene = nullptr;
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
      
  const std::vector<std::string>& SceneManager::ScenePaths() const {
    return scene_paths;
  }
      
  const std::map<UUID , SceneMetadata>& SceneManager::GetScenes() const {
    return loaded_scenes;
  }

  void SceneManager::UpdateScene(float dt) {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->Update(dt);
  }

  void SceneManager::RenderScene() {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->Render();
  }
      
  void SceneManager::RenderSceneUI() {
    if (!HasActiveScene()) {
      return;
    }

    active_scene->scene->RenderUI();
  }

} // namespace other
