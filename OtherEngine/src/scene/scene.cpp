/**
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "physics/physics_defines.hpp"
#include "scripting/script_engine.hpp"

#include "rendering/renderer.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/rigid_body_2d.hpp"

#include "ecs/systems/core_systems.hpp"

namespace other {

  Scene::Scene() {
    registry.on_construct<entt::entity>().connect<&OnConstructEntity>();
    registry.on_destroy<entt::entity>().connect<&OnDestroyEntity>();

    registry.on_construct<Camera>().connect<&OnCameraAddition>();

    registry.on_update<RigidBody2D>().connect<&OnRigidBody2DUpdate>();
  }

  Scene::~Scene() {
    registry.on_update<RigidBody2D>().disconnect<&OnRigidBody2DUpdate>();

    registry.on_construct<Camera>().disconnect<&OnCameraAddition>();

    registry.on_destroy<entt::entity>().disconnect<&OnDestroyEntity>();
    registry.on_construct<entt::entity>().disconnect<&OnConstructEntity>();

    for (auto& [id , entity] : entities) {
      delete entity;
    }
  }

  void Scene::Initialize() {
    FixRoots();

    registry.view<Script , Tag>().each([](Script& script , Tag& tag) {
      for (auto& [id , obj] : script.scripts) {
        obj->SetEntityId(tag.id);
        obj->OnBehaviorLoad();
        obj->Initialize();
      }
    });

    OnInit(); 
    initialized = true;
  }

  void Scene::Start() {
    OE_ASSERT(initialized , "Starting scene without initialization");

    if (physics_type == PHYSICS_2D) {
      registry.view<RigidBody2D , Tag , Transform>().each([&](RigidBody2D& body , const Tag& tag , const Transform& transform) {
        Initialize2DRigidBody(physics_world_2d , body , tag , transform);
      });
    }
    
    registry.view<Script>().each([](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->Start();
      }
    });

    registry.view<Camera>().each([](Camera& camera) {
      if (camera.camera->IsPrimary()) {
        Renderer::BindCamera(camera.camera);
      }
    });

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

    if (physics_type == PhysicsType::PHYSICS_2D) {
      OE_ASSERT(physics_world_2d != nullptr , "Can not step physics world with null scene (2D)!");

      /// TODO: fix these to customizeable
      physics_world_2d->Step(dt , 32 , 2);
    }

    registry.view<RigidBody2D , Transform>().each([](RigidBody2D& body , Transform& transform) {
      if (body.physics_body == nullptr) {
        return;
      } 

      auto& position = body.physics_body->GetPosition();
      transform.position.x = position.x;
      transform.position.y = position.y;
      transform.erotation.z = body.physics_body->GetAngle(); 
    });

    registry.view<Transform>().each([](Transform& transform) {
      transform.erotation = glm::eulerAngles(transform.qrotation);
      transform.model_transform = glm::translate(glm::mat4(1.f) , transform.position);
      transform.model_transform = glm::eulerAngleYXZ(
        transform.erotation.y , transform.erotation.x , transform.erotation.z
      );
      transform.model_transform = glm::scale(transform.model_transform , transform.scale);
    });

    registry.view<Script>().each([&dt](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->Update(dt);
      }
    });

    /// because every entity has a transform this is equivalent to view<Camera>
    registry.view<Camera, Transform>().each([](Camera& camera , Transform& transform) {
      if (camera.pinned_to_entity_position) {
        camera.camera->SetPosition(transform.position);
        camera.camera->SetOrientation(transform.erotation);
        camera.camera->CalculateMatrix();
      } else {
        camera.camera->CalculateMatrix();
      }
    });

    OnUpdate(dt);
    
    /// finish scene update
    /// checks the case the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }

  }
      
  void Scene::Render() {
    /// push active camera to bind UBOs
    /// set shader unidorms

    // registry.view<RenderedText>().each([](const RenderedText& mesh) {});

    registry.view<Mesh>().each([](const Mesh& mesh) {
      /// submit mesh to correct batch
    });
    
    registry.view<Script>().each([](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->Render();
      }
    });
  }
      
  void Scene::RenderUI() {
    // registry.view<UI>().each([](const UI& ui) {}); 
  }
      
  void Scene::LateUpdate(float dt) {
  }

  void Scene::Stop() {
    OE_ASSERT(initialized , "Updating scene without initialization");
    if (!running) {
      OE_WARN("Scene not running, nothing to stop");
      return;
    }
    
    registry.view<Script>().each([](Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->Stop();
      }
    });

    if (physics_type == PHYSICS_2D) {
      registry.view<RigidBody2D>().each([&](RigidBody2D& body) {
        physics_world_2d->DestroyBody(body.physics_body);
      });
    }

    running = false;
    OnStop();
  }

  void Scene::Shutdown() {
    OE_ASSERT(initialized , "Shutting down scene before initialization");
    initialized = false;
    OnShutdown();

    registry.view<Script>().each([](Script& script) {
      for (auto& [id , obj] : script.scripts) {
        obj->Shutdown();
        obj->OnBehaviorUnload();
      }
    });
  }
      
  void Scene::Refresh() {
    /// no refresh tag
    /// refresh transform
    /// refresh relationships?
    /// rebuild meshes

    registry.view<Script>().each([](Script& script) {
      script.scripts.clear();

      for (auto& [id , data] : script.data) {
        ScriptModule* mod = ScriptEngine::GetScriptModule(data.module);
        if (mod == nullptr) {
          OE_ERROR("Unable to reload script {}::{} : Failed to get Module" , data.module , data.obj_name);
          continue;
        }

        OE_DEBUG("Retrieving {} from {}" , data.obj_name , data.module);

        ScriptObject* inst = mod->GetScript(data.obj_name);
        if (inst == nullptr) {
          OE_ERROR("Unable to reload script {}::{} : Failed to get Object" , data.module , data.obj_name);
          continue;
        }

        script.scripts[id] = inst;
      }
    });
  }
      
  entt::registry& Scene::Registry() {
    return registry;
  }
      
  PhysicsType Scene::ActivePhysicsType() const {
    return physics_type;
  }
      
  Ref<PhysicsWorld2D> Scene::Get2DPhysicsWorld() const {
    return physics_world_2d;
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
      
  std::vector<BatchData> Scene::GetRenderBatchData() const {
    return {};
  }

  size_t Scene::NumCameras() const {
    return registry.view<Camera>().size();
  }
      
  const std::map<UUID , Entity*>& Scene::RootEntities() const {
    return root_entities;
  }

  const std::map<UUID , Entity*>& Scene::SceneEntities() const {
    return entities;
  }
      
  bool Scene::HasEntity(const std::string& name) const {
    return HasEntity(FNV(name));
  }
  
  bool Scene::HasEntity(UUID id) const {
    return entities.find(id) != entities.end();
  }
      
  Entity* Scene::GetEntity(const std::string& name) {
    auto ent = std::find_if(entities.begin() , entities.end() , [&name](const auto& ent_pair) -> bool {
      return name == ent_pair.second->Name();
    });

    if (ent == entities.end()) {
      return nullptr;
    }

    return ent->second;
  }
  
  Entity* Scene::GetEntity(UUID id) const {
    auto ent = entities.find(id);
    if (ent == entities.end()) {
      return nullptr;
    }

    return ent->second;
  }

  Entity* Scene::CreateEntity(const std::string& name) {
    std::string real_name = name;
    if (real_name.empty()) {
      real_name = fmtstr("[ Empty Object {}]" , entities.size());
    }

    UUID id = FNV(real_name);
    return CreateEntity(real_name , id);
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
      
  void Scene::RenameEntity(UUID curr_id , UUID new_id , const std::string_view name) {
    if (curr_id == new_id) {
      return;
    }

    auto itr = entities.find(curr_id);
    if (itr == entities.end()) {
      OE_ERROR("Can not rename entity with id [{}], it does not exist!" , curr_id);
      return;
    }

    Entity* entity = itr->second;
    auto& tag  = entity->GetComponent<Tag>();
    tag.name = name;
    tag.id = new_id;

    entities.erase(itr);

    entities[new_id] = entity;

    if (auto ritr = root_entities.find(curr_id); ritr != root_entities.end()) {
      root_entities.erase(ritr);
      root_entities[new_id] = entity;
    }
  }
      
  void Scene::ParentEntity(UUID id , UUID parent_id) {
    Entity* entity = GetEntity(id);
    Entity* parent = GetEntity(parent_id);

    if (entity == nullptr) {
      OE_ERROR("Attempting to parent a non-existent entity");
      return;
    }
    
    if (parent == nullptr) {
      OE_ERROR("Attempting to place entity as child of non-existent entity");
      return;
    }

    auto& crelationship = entity->GetComponent<Relationship>();
    auto& prelationship = parent->GetComponent<Relationship>();
    
    if (crelationship.parent.has_value()) {
      OrphanEntity(id);
    } 

    prelationship.children.insert(id);
    crelationship.parent = parent_id;

    FixRoots();
  }
      
  void Scene::OrphanEntity(UUID id) {
    Entity* entity = GetEntity(id);
    
    if (entity == nullptr) {
      OE_ERROR("Attempting to orphan non-existent entity");
      return;
    }

    auto& crelationship = entity->GetComponent<Relationship>();
    
    if (!crelationship.parent.has_value()) {
      return;
    }

    Entity* old_parent = GetEntity(crelationship.parent.value());

    if (old_parent == nullptr) {
      OE_ERROR("Entity relationships corrupt! {} has null parent" , entity->Name());
      return;
    }

    auto& prelation = old_parent->GetComponent<Relationship>();
    auto itr = std::find(prelation.children.begin() , prelation.children.end() , id);
    if (itr != prelation.children.end()) {
      prelation.children.erase(itr);
    }

    crelationship.parent = std::nullopt;

    FixRoots();
  }
      
  void Scene::FixRoots() {
    /// add any entities that should be root entities to roots
    for (auto itr = entities.begin(); itr != entities.end();) {
      if (!itr->second->GetComponent<Relationship>().parent.has_value()) {
        auto ritr = root_entities.find(itr->first);
        if (ritr == root_entities.end()) {
          root_entities[itr->first] = itr->second;
        }
      }
      ++itr;
    }

    /// remove any entities with a parent from root
    for (auto itr = root_entities.begin(); itr != root_entities.end();) {
      if (itr->second->GetComponent<Relationship>().parent.has_value()) {
        OE_DEBUG("De-rooting entity : {}" , itr->second->Name());
        itr = root_entities.erase(itr); 
      } else {
        ++itr;
      }
    }
  }

} // namespace other
