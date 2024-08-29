/**
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include <ranges>

#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "ecs/entity.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/camera.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/systems/core_systems.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  /// TODO: get rid of this in some nice ctor/dtor wrapper
  Scene::Scene() {
    registry.on_construct<entt::entity>().connect<&OnConstructEntity>();
    registry.on_destroy<entt::entity>().connect<&OnDestroyEntity>();

    registry.on_construct<Camera>().connect<&OnCameraAddition>();

    registry.on_construct<RigidBody2D>().connect<&Scene::OnAddRigidBody2D>(this);
    registry.on_update<RigidBody2D>().connect<&OnRigidBody2DUpdate>();
    
    registry.on_construct<Collider2D>().connect<&Scene::OnAddCollider2D>(this);
    registry.on_update<Collider2D>().connect<&OnCollider2DUpdate>();
    
    registry.on_construct<RigidBody>().connect<&Scene::OnAddRigidBody>(this);
    registry.on_update<RigidBody>().connect<&OnRigidBodyUpdate>();
    
    registry.on_construct<Collider>().connect<&Scene::OnAddCollider>(this);
    registry.on_update<Collider>().connect<&OnColliderUpdate>();
    
    registry.on_update<LightSource>().connect<&Scene::RebuildEnvironment>(this);
    registry.on_destroy<LightSource>().connect<&Scene::RebuildEnvironment>(this);

    registry.on_construct<Mesh>().connect<&OnAddModel>();
    registry.on_construct<StaticMesh>().connect<&OnAddStaticModel>();

    registry.on_construct<Mesh>().connect<&Scene::GeometryChanged>(this);
    registry.on_construct<StaticMesh>().connect<&Scene::GeometryChanged>(this);
    registry.on_update<Mesh>().connect<&Scene::GeometryChanged>(this);
    registry.on_update<StaticMesh>().connect<&Scene::GeometryChanged>(this);
    registry.on_destroy<Mesh>().connect<&Scene::GeometryChanged>(this);
    registry.on_destroy<StaticMesh>().connect<&Scene::GeometryChanged>(this);

    /// TODO: move this 
    environment = NewRef<Environment>();
  }

  Scene::~Scene() {
    registry.on_destroy<Mesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_destroy<StaticMesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_update<Mesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_update<StaticMesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_construct<Mesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_construct<StaticMesh>().disconnect<&Scene::GeometryChanged>(this);

    registry.on_construct<StaticMesh>().disconnect<&OnAddStaticModel>();
    registry.on_construct<Mesh>().disconnect<&OnAddModel>();

    registry.on_destroy<LightSource>().disconnect<&Scene::RebuildEnvironment>(this);
    registry.on_update<LightSource>().disconnect<&Scene::RebuildEnvironment>(this);
    
    registry.on_update<Collider>().disconnect();
    registry.on_construct<Collider>().disconnect();

    registry.on_update<RigidBody>().disconnect();
    registry.on_construct<RigidBody>().disconnect(this);

    registry.on_update<Collider2D>().disconnect();
    registry.on_construct<Collider2D>().disconnect();

    registry.on_update<RigidBody2D>().disconnect();
    registry.on_construct<RigidBody2D>().disconnect(this);

    registry.on_construct<Camera>().disconnect();

    registry.on_destroy<entt::entity>().disconnect();
    registry.on_construct<entt::entity>().disconnect();

    for (auto& [id , entity] : entities) {
      delete entity;
    }
  }

  void Scene::Initialize() {
    FixRoots();

    auto cs_module = ScriptEngine::GetModule(CS_MODULE);
    auto cs_core = cs_module->GetScriptModule("C#-Core");
    scene_object = cs_core->GetScript("Scene" , "Other");
    if (scene_object == nullptr) {
      OE_ERROR("Failed to retrieve scene interface from C# script core!");
    } else {
      scene_object->CallMethod("InitializeScene");
    }

    OE_DEBUG("Entities in scene [{}]" , entities.size()); 
    registry.view<Script , Tag>().each([](Script& script , Tag& tag) {
      for (auto& [id , obj] : script.scripts) {
        obj->Initialize();
      }
    });

    OnInit(); 
    initialized = true;
  }

  void Scene::Start() {
    OE_ASSERT(initialized , "Starting scene without initialization");
    FixRoots();

    registry.view<RigidBody2D , Tag , Transform>().each([this](RigidBody2D& body , const Tag& tag , const Transform& transform) {
      Initialize2DRigidBody(physics_world_2d , body , tag , transform);
    });
    
    registry.view<Script>().each([](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->Start();
      }
    });

    registry.view<Camera>().each([](Camera& camera) {
      DefaultUpdateCamera(camera.camera);
    });

    RebuildEnvironment();

    OnStart();
    running = true;
  }

  void Scene::EarlyUpdate(float dt) {
    OE_ASSERT(initialized , "Updating scene without initialization");
    if (!running) {
      return;
    }
    
    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }

    registry.view<Script>().each([&dt](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->EarlyUpdate(dt);
      }
    });
    
    OnEarlyUpdate(dt);
    
    /// finish scene update
    /// checks the case the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }
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

    /// TODO: check if simulating or just playing

    /**
     * order of updates
     *  - physics
     *      2d 
     *      3d
     *  - scripts, to pick up physics updates and apply script reactions
     *  - update transforms
     *  - apply transform updates to relevant components
     *   
     * use late update to react to other entity's changes
     **/

    if (physics_world_2d != nullptr) {
      physics_world_2d->Step(dt , 32 , 2);

      /// apply physics simulation to transforms, before using transforms for anything else 
      registry.view<RigidBody2D , Transform>().each([](RigidBody2D& body , Transform& transform) {
        if (body.physics_body == nullptr) {
          return;
        } 

        auto& position = body.physics_body->GetPosition();
        transform.position.x = position.x;
        transform.position.y = position.y;
        transform.erotation.z = body.physics_body->GetAngle(); 
      });
    }
    
    /// TODO: add 3d physics update here
    

    /// TODO: rigid body 3d here
   
    /// update environment
    registry.view<LightSource , Transform>().each([](LightSource& light , Transform& transform) {
      if (light.type == POINT_LIGHT_SRC) {
        transform.position = light.pointlight.position;
      } else {
      }
    });
    
    /// update transforms after other updates, dont overwrite physics changes
    registry.view<Transform>(entt::exclude<RigidBody2D , Collider2D , RigidBody , Collider>).each([](Transform& transform) {
      transform.CalcMatrix();
    });
    
    /// scripts updated last to give most accurate view of updated state 
    registry.view<Script>().each([&dt](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->Update(dt);
      }
    });
    
    /// update client app if they have custom logic ,
    ///   do this last to give client accurate state view
    OnUpdate(dt);
    
    /// finish scene update
    /// checks if the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }

  }
  
  void Scene::LateUpdate(float dt) {
    OE_ASSERT(initialized , "Updating scene without initialization");
    if (!running) {
      return;
    }
    
    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }
     
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

    registry.view<Script>().each([&dt](const Script& script) {
      for (auto& [id , s] : script.scripts) {
        s->LateUpdate(dt);
      }
    });
    
    OnLateUpdate(dt);
    
    /// finish scene update
    /// checks the case the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }
  }

  /**
   * first we'll have to set up the pipeline data for the scene
   *    void Scene:PreRender() {
   *      if (pipeline specs changed) {
   *        /// build pipeline specs
   *      }
   *    }
   *
   **/
      
  Ref<CameraBase> Scene::GetPrimaryCamera() const {
    Ref<CameraBase> primary_cam = nullptr;
    registry.view<Camera>().each([&primary_cam](const Camera& camera) {
      if (camera.camera->IsPrimary()) {
        primary_cam = camera.camera;
      }
    });

    return primary_cam;
  }
      
  void Scene::Render(Ref<SceneRenderer> renderer) {
    if (scene_geometry_changed) {
      renderer->ClearPipelines();

      registry.view<Camera>().each([&](const Camera& camera) {
        if (camera.camera->IsPrimary()) {
          renderer->SubmitCamera(camera.camera); 
        }
      });
    }
    
    renderer->SubmitEnvironment(environment);

    if (scene_geometry_changed) {
      registry.view<Mesh , Transform>().each([&renderer](const Mesh& mesh , const Transform& transform) {
        if (!AppState::Assets()->IsValid(mesh.handle)) {
          return;
        }
        
        auto model = AssetManager::GetAsset<Model>(mesh.handle);
        renderer->SubmitModel("Geometry" , model , transform.model_transform , mesh.material);
        renderer->SubmitModel("Debug" , model , transform.model_transform , mesh.material);
      });

      registry.view<StaticMesh , Transform>().each([&renderer](const StaticMesh& mesh , const Transform& transform) {
        if (!AppState::Assets()->IsValid(mesh.handle)) {
          return;
        }
        
        auto model = AssetManager::GetAsset<StaticModel>(mesh.handle);
        renderer->SubmitStaticModel("Geometry" , model , transform.model_transform , mesh.material);
        renderer->SubmitStaticModel("Debug" , model , transform.model_transform , mesh.material);
      });
    }
     
    // registry.view<Script>().each([](const Script& script) {
    //   for (auto& [id , s] : script.scripts) {
    //     s->Render();
    //   }
    // });

    DebugRender(renderer);
  }
      
  void Scene::DebugRender(Ref<SceneRenderer> renderer) {
    if (scene_geometry_changed) {
      scene_geometry_changed = false; 
    }
  }
      
  void Scene::RenderUI() {
    // registry.view<UI>().each([](const UI& ui) {}); 
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

    registry.view<RigidBody2D>().each([&](RigidBody2D& body) {
      physics_world_2d->DestroyBody(body.physics_body);
    });

    running = false;
    OnStop();

    if (scene_object == nullptr) {
      return;
    }

    scene_object->CallMethod("ClearObjects");
  }

  void Scene::Shutdown() {
    OE_ASSERT(initialized , "Shutting down scene before initialization");
    initialized = false;
    OnShutdown();

    registry.view<Script>().each([](Script& script) {
      for (auto& [id , obj] : script.scripts) {
        obj->Shutdown();
      }
    });

    scene_object = nullptr;
  }
      
  entt::registry& Scene::Registry() {
    return registry;
  }

  ScriptObject* Scene::SceneScriptObject() {
    return scene_object;
  }
      
  Ref<PhysicsWorld2D> Scene::Get2DPhysicsWorld() const {
    return physics_world_2d;
  }
      
  Ref<PhysicsWorld> Scene::GetPhysicsWorld() const {
    return physics_world;
  }

  Ref<Environment> Scene::GetEnvironment() const {
    return environment;
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
    auto itr = entities.find(id);
    if (itr != entities.end()) {
      do {
        id = id.Get() + 1;
        itr = entities.find(id);
      } while (itr != entities.end());
    }

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

  void Scene::DestroyEntity(UUID id) {
    auto ent_itr = entities.find(id);
    OE_ASSERT(ent_itr != entities.end() , "Somehow deleting non-existent entity [{}]" , id);

    auto& [eid , ent] = *ent_itr;

    /// TODO: make all children children of this entity's parents
    auto& relations = ent->GetComponent<Relationship>();
    for (auto& i : relations.children) {
      OrphanEntity(i);
    }

    OrphanEntity(id);
    
    FixRoots(); 
    
    entt::entity handle = ent->handle;
    delete ent;
    ent = nullptr;

    entities.erase(ent_itr);

    auto itr2 = root_entities.find(id);
    if (itr2 != root_entities.end()) {
      root_entities.erase(itr2);
    }
    

    registry.destroy(handle);
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

    if (entity->HasComponent<Script>()) {
      auto& scripts = entity->GetComponent<Script>();
      for (auto& [id , script] : scripts.scripts) {
        script->SetEntityId(new_id);
      } 
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
  
  void Scene::GeometryChanged() {
    scene_geometry_changed = true;
  }

  void Scene::RebuildEnvironment() {
    /// rebuild environment on light source change
    environment->point_lights.clear();
    environment->direction_lights.clear();
    registry.view<LightSource , Transform>().each([this](LightSource& light , Transform& transform) {
        switch (light.type) {
          case POINT_LIGHT_SRC:
            environment->point_lights.push_back(light.pointlight);
          break;
          case DIRECTION_LIGHT_SRC:
            environment->direction_lights.push_back(light.direction_light);
          break;
          default:
          break;
        }
    });
  }
      
  void Scene::OnAddRigidBody2D(entt::registry& context , entt::entity entt) {
    OE_ASSERT(physics_world_2d != nullptr , "Somehow created a rigid body 2D component without active 2D physics");

    Entity ent(context , entt);
    auto& body = ent.GetComponent<RigidBody2D>(); 

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();
    
    Initialize2DRigidBody(physics_world_2d , body , tag , transform); 
  }
      
  void Scene::OnAddCollider2D(entt::registry& context , entt::entity entt) {
    OE_ASSERT(physics_world_2d != nullptr , "Somehow created a collider 2D component without active 2D physics");

    Entity ent(context , entt);
    if (!ent.HasComponent<RigidBody2D>()) {
      ent.AddComponent<RigidBody2D>(); 
    }

    auto& body = ent.AddComponent<RigidBody2D>();
    auto& collider = ent.AddComponent<Collider2D>();
    auto& transform = ent.GetComponent<Transform>();

    Initialize2DCollider(physics_world_2d , body , collider , transform);
  }
      
  void Scene::OnAddRigidBody(entt::registry& context , entt::entity entt) {
    OE_ASSERT(physics_world != nullptr , "Somehow created a rigid body component without active 3D physics!");
    
    Entity ent(context , entt);
    auto& body = ent.GetComponent<RigidBody>(); 

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();
    
    InitializeRigidBody(physics_world , body , tag , transform); 
  }

  void Scene::OnAddCollider(entt::registry& context , entt::entity entt) {
    OE_ASSERT(physics_world != nullptr , "Somehow created a collider component without active 3D physics!");
    
    Entity ent(context , entt);
    if (!ent.HasComponent<RigidBody>()) {
      ent.AddComponent<RigidBody>(); 
    }

    auto& body = ent.AddComponent<RigidBody>();
    auto& collider = ent.AddComponent<Collider>();
    auto& transform = ent.GetComponent<Transform>();

    InitializeCollider(physics_world , body , collider , transform);
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
