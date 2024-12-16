/**
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include <ranges>

#include <entt/entity/entity.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <hosting/native_string.hpp>

#include "core/rand.hpp"

#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"

#include "ecs/components/camera.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"
#include "ecs/systems/core_systems.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/material.hpp"
#include "rendering/model.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/script_engine.hpp"

#include "editor/selection_manager.hpp"

namespace other {

  /// TODO: get rid of this in some nice ctor/dtor wrapper
  Scene::Scene()
      : Asset(), dotother::NObject(Random::Generate()) {
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

    environment = NewRef<LightEnvironment>();
    scene_handle = handle.Get();
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

    for (auto& [id, entity] : entities) {
      delete entity;
    }
  }

  const std::string& Scene::Name() const {
    return scene_name;
  }

  UUID Scene::SceneHandle() const {
    return scene_handle;
  }

  void Scene::Initialize() {
    FixRoots();

    scene_object = ScriptEngine::GetObjectRef<CsObject>("Scene", "Other", "OtherEngine.CsCore");
    OE_ASSERT(scene_object != nullptr, "Failed to retrieve scene object from script engine");
    scene_object->Initialize();

    OE_DEBUG("Setting Scene.NativeHandle([{:p}])", fmt::ptr(this));
    scene_object->SetHandles(scene_handle, entt::null, this);

    OE_DEBUG("Entities in scene [{}]", entities.size());
    registry.view<Script, Tag>().each([this](Script& script, Tag& tag) {
      script.ApiCall("NativeInitialize");
      script.ApiCall("OnInitialize");
    });

    registry.view<Transform>().each([](Transform& transform) {
      transform.CalcMatrix();
    });

    RefreshCameraTransforms();

    OnInit();
    initialized = true;

    BuildGroups();
    RebuildEnvironment();

    /// update so all transforms are calculate and all cross depenedent components are updated accordingly
    EarlyUpdate(0.f);
    Update(0.f);
    LateUpdate(0.f);
  }

  bool Scene::IsHandleValid(Entity* ent) const {
    if (ent == nullptr) {
      return false;
    }

    UUID id = ent->GetUUID();
    auto itr = entities.find(id);
    return itr != entities.end();
  }

  void Scene::Shutdown() {
    OE_ASSERT(initialized, "Shutting down scene before initialization");
    initialized = false;

    if (running) {
      Stop();
    }

    OnShutdown();

    registry.view<Script>().each([](Script& script) {
      script.ApiCall("OnShutdown");
      script.ApiCall("NativeShutdown");
    });

    scene_object->Shutdown();
    scene_object = nullptr;
  }

  void Scene::Start(EngineMode mode) {
    OE_ASSERT(initialized, "Starting scene without initialization");

    FixRoots();

    registry.view<RigidBody2D, Tag, Transform>().each([this](RigidBody2D& body, const Tag& tag, const Transform& transform) {
      Initialize2DRigidBody(physics_world_2d, body, tag, transform);
    });

    registry.view<Script>().each([&](Script& script) {
      script.ApiCall("NativeStart");
      script.ApiCall("OnStart");
    });

    scene_object->Start();

    RefreshCameraTransforms();

    OnStart();

    /// do this after client in case the modify environment
    RebuildEnvironment();

    running = true;
  }

  void Scene::Stop() {
    OE_ASSERT(initialized, "Updating scene without initialization");
    if (!running) {
      return;
    }
    running = false;

    registry.view<Script>().each([](Script& script) {
      script.ApiCall("OnStop");
      script.ApiCall("NativeStop");
    });

    registry.view<RigidBody2D>().each([&](RigidBody2D& body) {
      physics_world_2d->DestroyBody(body.physics_body);
    });

    scene_object->Stop();

    OnStop();

    if (scene_object == nullptr) {
      return;
    }

    // scene_object->CallMethod("ClearObjects");
  }

  void Scene::EarlyUpdate(float dt) {
    OE_ASSERT(initialized, "Updating scene without initialization");
    if (!running) {
      return;
    }

    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }

    registry.view<Script>().each([&dt](Script& script) {
      script.ApiCall("EarlyUpdate", dt);
    });

    scene_object->EarlyUpdate(dt);

    OnEarlyUpdate(dt);

    /// finish scene update
    /// checks the case the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }
  }

  void Scene::Update(float dt) {
    OE_ASSERT(initialized, "Updating scene without initialization");
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
      physics_world_2d->Step(dt, 32, 2);

      /// apply physics simulation to transforms, before using transforms for anything else
      registry.view<RigidBody2D, Transform>().each([](RigidBody2D& body, Transform& transform) {
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

    /// update transforms after other updates, dont overwrite physics changes
    registry.view<Transform>(entt::exclude<RigidBody2D, Collider2D, RigidBody, Collider>).each([](Transform& transform) {
      transform.CalcMatrix();

      /// update quaternion
      transform.qrotation = glm::quat(transform.erotation);
      glm::quat conj = glm::conjugate(transform.qrotation);

      glm::vec3 dim = transform.scale * 0.5f;
      glm::vec4 min = glm::vec4(transform.position - dim, 1.f);
      glm::vec4 max = glm::vec4(transform.position - dim, 1.f);

      glm::vec3 rotated_min = glm::vec3(transform.qrotation * min * conj);
      glm::vec3 rotated_max = glm::vec3(transform.qrotation * max * conj);

      BBox bounding_box = BBox(rotated_min, rotated_max);
      transform.bbox = bounding_box;
    });

    /// scripts updated last to give most accurate view of updated state
    registry.view<Script>().each([&dt](Script& script) {
      script.ApiCall("Update", dt);
    });

    scene_object->Update(dt);

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
    OE_ASSERT(initialized, "Updating scene without initialization");

    /// update environment even if scene is not running
    registry.view<LightSource, Transform>().each([&](LightSource& light, Transform& transform) {
      environment->direction_light = std::nullopt;
      environment->point_lights.clear();

      glm::mat4 eye = glm::mat4(1.f);
      if (light.type == DIRECTION_LIGHT_SRC) {
        /// sync direction light transforms to light data
        transform.erotation = glm::vec3(light.direction_light.direction);
        transform.position = -glm::normalize(glm::vec3(light.direction_light.direction)) * 10.f;
        light.direction_light.position = glm::vec4(transform.position, 1.0);

        float near_plane = 0.1f, far_plane = 100.f;
        // glm::vec3 light_target = transform.position + glm::vec3(light.direction_light.direction);
        glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 light_view = glm::lookAt(transform.position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        light.direction_light.light_space_matrix = light_projection * light_view;

      } else if (light.type == POINT_LIGHT_SRC) {
        /// sync pointlight transforms to pointlight data
        transform.position = light.pointlight.position;
        transform.scale = glm::vec3(0.2f);

        // light.pointlight.light_space_matrix = glm::translate(eye, light.pointlight.position);
      }

      if (light.type == DIRECTION_LIGHT_SRC && !environment->direction_light.has_value()) {
        environment->direction_light = light.direction_light;
      } else if (light.type == POINT_LIGHT_SRC) {
        environment->point_lights.push_back(light.pointlight);
      }
    });

    /// update material tables if they have changed
    std::set<UUID> model_sources;
    registry.view<Mesh>().each([&model_sources](Mesh& mesh) {
      if (!AppState::Assets()->IsValid(mesh.handle)) {
        return;
      }

      auto model = AssetManager::GetAsset<Model>(mesh.handle);
      if (model == nullptr) {
        return;
      }

      model_sources.insert(model->GetModelSource()->handle.Get());
    });
    registry.view<StaticMesh>().each([&model_sources](StaticMesh& mesh) {
      if (!AppState::Assets()->IsValid(mesh.handle)) {
        return;
      }

      auto model = AssetManager::GetAsset<StaticModel>(mesh.handle);
      if (model == nullptr) {
        return;
      }

      model_sources.insert(model->GetModelSource()->handle.Get());
    });

    uint32_t layers = std::numeric_limits<uint32_t>::min();
    for (UUID model_src : model_sources) {
      Ref<ModelSource> src = AssetManager::GetAsset<ModelSource>(model_src);
      if (src == nullptr) {
        continue;
      }

      layers = layers > src->SubMeshes().size() ?
        layers :
        src->SubMeshes().size();
    }
    if (layers != std::numeric_limits<uint32_t>::min()) {
      material_table->Recreate(1, layers, glm::vec2{ 1920.f, 1080.f });
      registry.view<Mesh>().each([&](Mesh& mesh) {
        if (!AppState::Assets()->IsValid(mesh.handle)) {
          return;
        }

        auto model = AssetManager::GetAsset<Model>(mesh.handle);
        if (model == nullptr) {
          return;
        }

        Ref<MaterialTable> src_mat_table = model->GetModelSource()->GetMaterialTable();
        OE_ASSERT(src_mat_table != nullptr, "Model has no material table");

        std::vector<uint32_t> submeshes = model->SubMeshes();
        for (uint32_t sm_idx : submeshes) {
          if (sm_idx >= model->GetModelSource()->SubMeshes().size()) {
            continue;
          }
          SubMesh& submesh = model->GetModelSource()->SubMeshes()[sm_idx];
          Material& mat = submesh.material;

          material_table->SetTexture(MaterialTable::ALBEDO, sm_idx, src_mat_table->GetTexturePixels(MaterialTable::ALBEDO, mat.albedo_tex_idx));
          material_table->SetTexture(MaterialTable::NORMAL, sm_idx, src_mat_table->GetTexturePixels(MaterialTable::NORMAL, mat.normal_tex_idx));
          material_table->SetTexture(MaterialTable::ROUGHNESS, sm_idx, src_mat_table->GetTexturePixels(MaterialTable::ROUGHNESS, mat.roughness_tex_idx));
        }
      });
      registry.view<StaticMesh>().each([&](StaticMesh& mesh) {
        if (!AppState::Assets()->IsValid(mesh.handle)) {
          return;
        }

        Ref<MaterialTable> src_mat_table = model->GetModelSource()->GetMaterialTable();
        OE_ASSERT(src_mat_table != nullptr, "Model has no material table");

        std::vector<SubMesh> submeshes = model->GetModelSource()->SubMeshes();
        for (uint32_t i = 0; i < submeshes.size(); ++i) {
          SubMesh& sm = submeshes[i];
          Material& mat = sm.material;

          material_table->SetTexture(MaterialTable::ALBEDO, i, src_mat_table->GetTexturePixels(MaterialTable::ALBEDO, mat.albedo_tex_idx));
          material_table->SetTexture(MaterialTable::NORMAL, i, src_mat_table->GetTexturePixels(MaterialTable::NORMAL, mat.normal_tex_idx));
          material_table->SetTexture(MaterialTable::ROUGHNESS, i, src_mat_table->GetTexturePixels(MaterialTable::ROUGHNESS, mat.roughness_tex_idx));
        }
      });
    }
    OE_ASSERT(layers != std::numeric_limits<uint32_t>::min(), "No models in scene");

    if (!running) {
      return;
    }

    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }

    registry.view<Camera, Transform>().each([](Camera& camera, Transform& transform) {
      OE_ASSERT(camera.camera != nullptr, "Camera is null");  /// should never happen
      if (camera.pinned_to_entity_position) {
        camera.camera->SetPosition(transform.position);
      }

      DefaultUpdateCamera(camera.camera);
    });

    registry.view<Script>().each([&dt](Script& script) {
      script.ApiCall("LateUpdate", dt);
    });

    scene_object->LateUpdate(dt);

    OnLateUpdate(dt);

    /// finish scene update
    /// checks the case the scene become corrupt on client update
    if (corrupt) {
      Stop();
      return;
    }
  }

  Ref<CameraBase> Scene::GetPrimaryCamera() const {
    UUID handle = 0;
    registry.view<Camera, Tag>().each([&handle](const Camera& camera, const Tag& tag) {
      if (handle.Get() != 0) {
        return;
      }
      if (camera.is_primary) {
        handle = tag.id;
      }
    });

    if (handle.Get() == 0) {
      // OE_WARN("No primary camera found in scene");
      return nullptr;
    }

    return GetEntity(handle)->GetComponent<Camera>().camera;
  }

  void Scene::Render(Ref<SceneRenderer>& renderer) {
    OnRender();

    if (auto primary_cam = GetPrimaryCamera(); primary_cam != nullptr) {
      renderer->SubmitCamera(primary_cam);
    }

    // if (scene_geometry_changed) {
    //   RebuildEnvironment();
    //   scene_geometry_changed = false;
    //   renderer->SubmitEnvironment(environment);
    // }
    renderer->SubmitEnvironment(environment);
    renderer->SubmitMaterialTable(material_table);
    dynamic_mesh_group.each([&renderer](const Mesh& mesh, const Transform& transform) {
      if (!AppState::Assets()->IsValid(mesh.handle)) {
        return;
      }

      auto model = AssetManager::GetAsset<Model>(mesh.handle);
      renderer->SubmitModel(model, transform.model_transform, mesh.materials);
    });

    static_mesh_group.each([&renderer](const StaticMesh& mesh, const Transform& transform) {
      if (!AppState::Assets()->IsValid(mesh.handle)) {
        return;
      }

      auto model = AssetManager::GetAsset<StaticModel>(mesh.handle);
      renderer->SubmitStaticModel(model, transform.model_transform, mesh.material);
    });

    // AssetHandle cube_handle = ModelFactory::CreateBox();

    // light_group.each([&renderer, cube_handle, plname](const LightSource& light, const Transform& transform) {
    //   if (light.type == DIRECTION_LIGHT_SRC) {
    //     return;
    //   }

    //   if (!AppState::Assets()->IsValid(cube_handle)) {
    //     return;
    //   }

    //   Material light_material = Material(light.pointlight.color, 32.f);

    //   auto model = AssetManager::GetAsset<StaticModel>(cube_handle);
    //   renderer->SubmitStaticModel(plname, model, transform.model_transform, light_material);
    // });

    scene_object->Render();
  }

  void Scene::RenderUI() {
    // registry.view<UI>().each([](const UI& ui) {});
    scene_object->RenderUI();
  }

  entt::registry& Scene::Registry() {
    return registry;
  }

  ScriptRef<CsObject> Scene::SceneScriptObject() {
    return scene_object;
  }

  Ref<PhysicsWorld2D> Scene::Get2DPhysicsWorld() const {
    return physics_world_2d;
  }

  Ref<PhysicsWorld> Scene::GetPhysicsWorld() const {
    return physics_world;
  }

  Ref<LightEnvironment> Scene::GetEnvironment() const {
    return environment;
  }

  Ref<MaterialTable> Scene::GetMaterialTable() const {
    return material_table;
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
    return std::find_if(entities.begin(), entities.end(), [&id](const auto& ent_pair) -> bool {
             return id == ent_pair.first;
           }) != entities.end();
  }

  bool Scene::EntityExists(const std::string& name) const {
    return EntityExists(FNV(name));
  }

  size_t Scene::NumCameras() const {
    return registry.view<Camera>().size();
  }

  const std::map<UUID, Entity*>& Scene::RootEntities() const {
    return root_entities;
  }

  const std::map<UUID, Entity*>& Scene::SceneEntities() const {
    return entities;
  }

  bool Scene::HasEntity(const std::string& name) const {
    return HasEntity(FNV(name));
  }

  bool Scene::HasEntity(UUID id) const {
    return entities.find(id) != entities.end();
  }

  Entity* Scene::GetEntity(const std::string& name) {
    auto ent = std::find_if(entities.begin(), entities.end(), [&name](const auto& ent_pair) -> bool {
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

    OE_ASSERT(ent->second != nullptr, "Entity with id [{}] is null", id);
    return ent->second;
  }

  Entity* Scene::CreateEntity(const std::string& name) {
    std::string real_name = name;
    if (real_name.empty()) {
      real_name = fmtstr("[ Empty Object {}]", entities.size());
    }

    UUID id = FNV(real_name);
    auto itr = entities.find(id);
    if (itr != entities.end()) {
      do {
        id = id.Get() + 1;
        itr = entities.find(id);
      } while (itr != entities.end());
    }

    return CreateEntity(real_name, id);
  }

  Entity* Scene::CreateEntity(const std::string& name, UUID id) {
    Entity* ent = new Entity(registry, id, name);
    for (const auto& [eid, e] : entities) {
      if (eid == id && e->Name() == ent->Name()) {
        OE_WARN("Entity[{} : {}] already exists in scene", id, e->Name());
        return nullptr;
      }
    }

    OE_ASSERT(ent != nullptr, "Failed to create entity [{}]", id);
    OE_ASSERT(ent->HasComponent<Tag>(), "Entity does not have tag component");

    root_entities[id] = ent;
    entities[id] = ent;

    return ent;
  }

  void Scene::DestroyEntity(UUID id) {
    auto ent_itr = entities.find(id);
    OE_ASSERT(ent_itr != entities.end(), "Somehow deleting non-existent entity [{}]", id);

    auto& [eid, ent] = *ent_itr;

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

  void Scene::RenameEntity(UUID curr_id, UUID new_id, const std::string_view name) {
    if (curr_id == new_id) {
      return;
    }

    auto itr = entities.find(curr_id);
    if (itr == entities.end()) {
      OE_ERROR("Can not rename entity with id [{}], it does not exist!", curr_id);
      return;
    }

    Entity* entity = itr->second;
    auto& tag = entity->GetComponent<Tag>();
    tag.name = name;
    tag.id = new_id;

    entities.erase(itr);

    entities[new_id] = entity;

    if (auto ritr = root_entities.find(curr_id); ritr != root_entities.end()) {
      root_entities.erase(ritr);
      root_entities[new_id] = entity;
    }

    if (entity->HasComponent<Script>()) {
      // auto& scripts = entity->GetComponent<Script>();
      // for (auto& [id , script] : scripts.scripts) {
      //   script->SetEntityId(new_id);
      // }
    }
  }

  void Scene::ParentEntity(UUID id, UUID parent_id) {
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
      OE_ERROR("Entity relationships corrupt! {} has null parent", entity->Name());
      return;
    }

    auto& prelation = old_parent->GetComponent<Relationship>();
    auto itr = std::find(prelation.children.begin(), prelation.children.end(), id);
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
    environment->direction_light = std::nullopt;
    environment->point_lights.clear();
    registry.view<LightSource, Transform>().each([this](LightSource& light, Transform& transform) {
      switch (light.type) {
        case POINT_LIGHT_SRC:
          environment->point_lights.push_back(light.pointlight);
          break;
        case DIRECTION_LIGHT_SRC:
          if (!environment->direction_light.has_value()) {
            environment->direction_light = light.direction_light;
          }
          break;
        default:
          break;
      }
    });
  }

  void Scene::OnAddRigidBody2D(entt::registry& context, entt::entity entt) {
    OE_ASSERT(physics_world_2d != nullptr, "Somehow created a rigid body 2D component without active 2D physics");

    Entity ent(context, entt);
    auto& body = ent.GetComponent<RigidBody2D>();

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();

    Initialize2DRigidBody(physics_world_2d, body, tag, transform);
  }

  void Scene::OnAddCollider2D(entt::registry& context, entt::entity entt) {
    OE_ASSERT(physics_world_2d != nullptr, "Somehow created a collider 2D component without active 2D physics");

    Entity ent(context, entt);
    if (!ent.HasComponent<RigidBody2D>()) {
      ent.AddComponent<RigidBody2D>();
    }

    auto& body = ent.AddComponent<RigidBody2D>();
    auto& collider = ent.AddComponent<Collider2D>();
    auto& transform = ent.GetComponent<Transform>();

    Initialize2DCollider(physics_world_2d, body, collider, transform);
  }

  void Scene::OnAddRigidBody(entt::registry& context, entt::entity entt) {
    OE_ASSERT(physics_world != nullptr, "Somehow created a rigid body component without active 3D physics!");

    Entity ent(context, entt);
    auto& body = ent.GetComponent<RigidBody>();

    auto& tag = ent.GetComponent<Tag>();
    auto& transform = ent.GetComponent<Transform>();

    InitializeRigidBody(physics_world, body, tag, transform);
  }

  void Scene::OnAddCollider(entt::registry& context, entt::entity entt) {
    OE_ASSERT(physics_world != nullptr, "Somehow created a collider component without active 3D physics!");

    Entity ent(context, entt);
    if (!ent.HasComponent<RigidBody>()) {
      ent.AddComponent<RigidBody>();
    }

    auto& body = ent.AddComponent<RigidBody>();
    auto& collider = ent.AddComponent<Collider>();
    auto& transform = ent.GetComponent<Transform>();

    InitializeCollider(physics_world, body, collider, transform);
  }

  void Scene::RefreshCameraTransforms() {
    registry.view<Camera>().each([](Camera& camera) {
      other::DefaultUpdateCamera(camera.camera);
    });
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
        OE_DEBUG("De-rooting entity : {}", itr->second->Name());
        itr = root_entities.erase(itr);
      } else {
        ++itr;
      }
    }
  }

  void Scene::BuildGroups() {
    connection_group = GetGroup<Relationship>();
    light_group = GetGroup<LightSource, Transform>();
    script_group = GetGroup<Script>();

    dynamic_mesh_group = GetGroup<Mesh, Transform>();
    static_mesh_group = GetGroup<StaticMesh, Transform>();
  }

}  // namespace other
