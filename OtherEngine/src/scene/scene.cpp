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

  void Scene::AddEntity(const std::string& name) {
    UUID id = FNV(name);
    Entity* ent = new Entity(this , id , name);
    AddEntity(ent);
  }
      
  void Scene::AddEntity(Entity* entity) {
    if (entity == nullptr) {
      OE_ERROR("Attempting to add null entity to scene");
      return;
    }

    auto& tag = entity->GetComponent<Tag>();
    for (const auto& [id , ent] : entities) {
      if (tag.id == id && tag.name == ent->Name()) {
        OE_WARN("Entity {} [{}] already exists in scene" , tag.name , id);
        return;
      }
    }

    entities[tag.id] = entity;
  }

} // namespace other
