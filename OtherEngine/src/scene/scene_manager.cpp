/**
 * \file scene/scene_manager.cpp
 **/
#include "scene/scene_manager.hpp"

#include <algorithm>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "ecs/entity.hpp"
#include "scene/scene_serializer.hpp"

namespace other {

  bool SceneManager::LoadScene(const Path& scenepath) {
    UUID id = FNV(scenepath.string());
    if (auto scn = loaded_scenes.find(id); scn != loaded_scenes.end()) {
      active_scene = &scn->second;
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
    }

    active_scene = &loaded_scenes[id];
  }

  bool SceneManager::HasScene(const Path& path) {
    return loaded_scenes.find(FNV(path.string())) != loaded_scenes.end();
  }

  const SceneMetadata* SceneManager::ActiveScene() const { 
    if (active_scene == nullptr) {
      return nullptr;
    }
    return active_scene;
  }

  void SceneManager::UnloadActive() {
    active_scene->scene->Stop();
    active_scene = nullptr;
  }
      
  const std::vector<std::string>& SceneManager::ScenePaths() const {
    return scene_paths;
  }
      
  const std::map<UUID , SceneMetadata>& SceneManager::GetScenes() const {
    return loaded_scenes;
  }
      
  Entity* SceneManager::BuildEntityFromConfigTable(Ref<Scene>& ctx , const std::string& name , const ConfigTable& table) const {
    /// check if there is a UUID in scene table otherwise we hash a name 
    UUID id = table.GetVal<UUID>(name , "UUID").value_or(FNV(name));
    Entity* ent = new Entity(ctx , id , name); 

    auto components = table.Get(name , "COMPONENTS");
    for (auto& comp_key : components) {
      std::string real_key = name + "." + comp_key;
      std::ranges::transform(real_key.begin() , real_key.end() , real_key.begin() , ::toupper);

      // auto derializer = GetComponentSerializer(real_key);
      // deserializer->Deserialize(entity , table);

      OE_DEBUG("{} has component : {}" , name , real_key);
    }

    return ent;
  }

} // namespace other
