/**scene.c
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/systems/core_systems.hpp"

namespace other {

  Scene::Scene() {
    registry.on_construct<entt::entity>().connect<&OnConstructEntity>();
    registry.on_destroy<entt::entity>().connect<&OnDestroyEntity>();
  }

  Scene::~Scene() {
    registry.on_destroy<entt::entity>().disconnect<&OnDestroyEntity>();
    registry.on_construct<entt::entity>().disconnect<&OnConstructEntity>();

    for (auto& [id , entity] : entities) {
      delete entity;
    }
  }

  void Scene::Start() {
    OnStart();
  }

  void Scene::Stop() {
    OnStop();
  }

  const std::map<UUID , Entity*>& Scene::SceneEntities() const {
    return entities;
  }
  
  const Entity* Scene::GetEntity(UUID id) const {
    auto ent = entities.find(id);
    if (ent == entities.end()) {
      return nullptr;
    }

    return ent->second;
  }

  Entity* Scene::CreateEntity(const std::string& name) {
    UUID id = FNV(name);
    return CreateEntity(name , id);
  }

  Entity* Scene::CreateEntity(const std::string& name , UUID id) {
    Entity* ent = new Entity(this , id , name); 
    for (const auto& [eid , e] : entities) {
      if (eid == id && e->Name() == ent->Name()) {
        OE_WARN("Entity[{} : {}] already exists in scene" , id , e->Name());
        return nullptr;
      }
    }

    entities[id] = ent;
    
    auto& tag = ent->GetComponent<Tag>();
    tag.id = id;
    tag.name = name;

    return ent;
  }

} // namespace other
