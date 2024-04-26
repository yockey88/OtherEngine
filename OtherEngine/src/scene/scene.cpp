/**
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include <algorithm>
#include <filesystem>

#include "core/rand.hpp"
#include "core/errors.hpp"
#include "core/logger.hpp"
#include "parsing/ini_parser.hpp"
#include "ecs/entity.hpp"

namespace other {

  Scene::Scene(const Path& path) 
      : scene_path(path) {
    LoadScene(); 
  }
      
  Scene::~Scene() {
    for (auto& [id , entity] : entities) {
      delete entity;
    }
  }

  const std::map<UUID , Entity*>& Scene::SceneEntities() const {
    return entities;
  }

  void Scene::LoadScene() {
    try {
      IniFileParser parser{ scene_path.string() };
      scene_table = parser.Parse();

      auto entities = scene_table.Get("METADATA" , "ENTITIES");
      OE_DEBUG("Scene {} has {} entities" , scene_path , entities.size());
      for (auto& e : entities) {
        OE_DEBUG("Loading Entity : {}" , e);
        AddEntity(e);
      }

      corrupt = false;

      OE_INFO("Scene loaded : {} (Entites : {})" , scene_path , entities.size());
    } catch (IniException& err) {
      OE_WARN("Failed to parse scene file - {} : {}" , scene_path , err.what()); 
    }  
  }
  
  void Scene::AddEntity(const std::string& name) {
    auto n = name;
    std::transform(n.begin() , n.end() , n.begin() , ::toupper);
    auto uuid_opt = scene_table.GetVal<uint64_t>(n , "UUID");
    UUID id = UUID(uuid_opt.value_or(Random::Generate()));

    entities[id] = new Entity(&registry , registry.create() , id , name);
  }

} // namespace other
