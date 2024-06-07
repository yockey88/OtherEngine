/**scene.c
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/relationship.hpp"
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

  void Scene::Initialize() {
    /// check if any entities were given a parent and need to removed from root
    for (auto itr = root_entities.begin(); itr != root_entities.end();) {
      if (itr->second->GetComponent<Relationship>().parent.has_value()) {
        OE_DEBUG("De-rooting entity : {}" , itr->second->Name());
        itr = root_entities.erase(itr); 
      } else {
        ++itr;
      }
    }

    OnInit(); 
    initialized = true;
  }

  void Scene::Start() {
    OE_ASSERT(initialized , "Starting scene without initialization");

    OnStart();
    running = true;
  }

  void Scene::Update(float dt) {
    OE_ASSERT(initialized , "Updating scene without initialization");
    if (!running) {
      return;
    }

    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }

    OnUpdate(dt);
    
    /// finish scene update
    /// checks the case the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }

  }

  void Scene::Stop() {
    OE_ASSERT(initialized , "Updating scene without initialization");
    if (!running) {
      OE_WARN("Scene not running, nothing to stop");
      return;
    }

    running = false;
    OnStop();
  }

  void Scene::Shutdown() {
    OE_ASSERT(initialized , "Shutting down scene before initialization");
    initialized = false;
    OnShutdown();
  }

  const bool Scene::IsInitialized() const {
    return initialized;
  }

  const bool Scene::IsRunning() const {
    return running;
  }

  const bool Scene::IsDirty() const {
    return corrupt;
  }

  bool Scene::EntityExists(UUID id) const {
    return std::find_if(entities.begin() , entities.end() , [&id](const auto& ent_pair) ->bool {
      return id == ent_pair.first; 
    }) != entities.end();
  }

  bool Scene::EntityExists(const std::string& name) const {
    return std::find_if(entities.begin() , entities.end() , [&name](const auto& ent_pair) ->bool {
      return name == ent_pair.second->Name(); 
    }) != entities.end();
  }
      
  const std::map<UUID , Entity*>& Scene::RootEntities() const {
    return root_entities;
  }

  const std::map<UUID , Entity*>& Scene::SceneEntities() const {
    return entities;
  }
  
  Entity* Scene::GetEntity(UUID id) const {
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

    root_entities[id] = ent;
    entities[id] = ent;
    
    auto& tag = ent->GetComponent<Tag>();
    tag.id = id;
    tag.name = name;

    return ent;
  }

} // namespace other
