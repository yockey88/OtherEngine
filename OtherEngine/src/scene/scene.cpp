/**
 * \file scene/scene.cpp
 **/
#include "scene/scene.hpp"

#include <ranges>

#include <box2d/b2_types.h>
#include <entt/entity/entity.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <hosting/native_string.hpp>

#include "core/logger.hpp"
#include "core/rand.hpp"

#include "application/app_state.hpp"
#include "asset/asset_manager.hpp"
#include "input/mouse.hpp"

#include "ecs/components/camera.hpp"
#include "ecs/components/collider_2d.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/physics_component.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/rigid_body_2d.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/tag.hpp"
#include "ecs/components/terrain.hpp"
#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"
#include "ecs/systems/core_systems.hpp"

#include "physics/physics_defines.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/renderer.hpp"
#include "rendering/vertex.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/script_engine.hpp"

#include "editor/selection_manager.hpp"

namespace other {

  ArenaAllocator<Entity> Scene::entity_allocator;

  /// TODO: get rid of this in some nice ctor/dtor wrapper
  Scene::Scene()
      : Asset(), dotother::NObject(handle.Get()) {
    registry.on_construct<entt::entity>().connect<&OnConstructEntity>();
    registry.on_destroy<entt::entity>().connect<&OnDestroyEntity>();

    registry.on_construct<Camera>().connect<&OnCameraAddition>();

    registry.on_update<Transform>().connect<&Scene::OnUpdateTransform>(this);

    registry.on_construct<PhysicsObject>().connect<&Scene::OnAddPhysicsObject>(this);
    registry.on_destroy<PhysicsObject>().connect<&Scene::OnDestroyPhysicsObject>(this);

    registry.on_construct<Terrain>().connect<&Scene::OnAddTerrain>(this);

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

    scene_entity = CreateEntity("Scene");
  }

  Scene::~Scene() {
    registry.on_destroy<Mesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_destroy<StaticMesh>().disconnect<&Scene::GeometryChanged>(this);

    registry.on_update<Transform>().disconnect<&Scene::OnUpdateTransform>(this);

    registry.on_update<Mesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_update<StaticMesh>().disconnect<&Scene::GeometryChanged>(this);

    registry.on_construct<Mesh>().disconnect<&Scene::GeometryChanged>(this);
    registry.on_construct<StaticMesh>().disconnect<&Scene::GeometryChanged>(this);

    registry.on_construct<StaticMesh>().disconnect<&OnAddStaticModel>();
    registry.on_construct<Mesh>().disconnect<&OnAddModel>();

    registry.on_destroy<LightSource>().disconnect<&Scene::RebuildEnvironment>(this);
    registry.on_update<LightSource>().disconnect<&Scene::RebuildEnvironment>(this);

    registry.on_construct<PhysicsObject>().disconnect<&Scene::OnAddPhysicsObject>(this);
    registry.on_destroy<PhysicsObject>().disconnect<&Scene::OnDestroyPhysicsObject>(this);

    registry.on_construct<Terrain>().disconnect<&Scene::OnAddTerrain>(this);

    registry.on_construct<Camera>().disconnect<&OnCameraAddition>();

    registry.on_update<Collider2D>().disconnect();
    registry.on_construct<Collider2D>().disconnect();

    registry.on_update<RigidBody2D>().disconnect();
    registry.on_construct<RigidBody2D>().disconnect(this);

    registry.on_construct<Camera>().disconnect();

    registry.on_destroy<entt::entity>().disconnect();
    registry.on_construct<entt::entity>().disconnect();

    for (auto& [id, entity] : entities) {
      entity_allocator.Free(entity);
    }
  }

  const std::string& Scene::Name() const {
    return scene_name;
  }

  UUID Scene::SceneHandle() const {
    return scene_handle;
  }

  void Scene::Initialize() {
    Script& scene_object = scene_entity->AddComponent<Script>();
    scene_object.parent_handle = scene_entity;
    scene_object.parent_uuid = scene_handle;
    scene_object.parent_id = scene_entity->Handle();
    scene_object.AddScript("Scene", "Other", "OtherEngine.CsCore");

    FixRoots();
    Synchronize();
    BuildGroups();

    OE_DEBUG("Entities in scene [{}]", entities.size());

    registry.view<Script, Tag>().each([this](Script& script, Tag& tag) {
      script.ApiCall("NativeInitialize");
      script.ApiCall("OnInitialize");
    });

    OnInit();
    initialized = true;

    RebuildEnvironment();
  }

  void Scene::Activate() {
    OE_ASSERT(initialized, "Activating scene without initialization");
    OE_TRACE("Activating Scene : {}", scene_name);

    /// Do this here and in Start because in the editor we should have all loaded entities set here
    ///   and then if user adds more entities before starting scene we add them in Start
    Script& scene_object = scene_entity->GetComponent<Script>();
    for (auto& [id, entity] : entities) {
      scene_object.ApiCall<uint64_t>("RegisterSceneObject", id.Get());
    }

    OnActivate();
    active = true;
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

    Script& scene_object = scene_entity->GetComponent<Script>();
    scene_object.ApiCall("ClearObjects");
    scene_object.RemoveScript();

    scene_entity->RemoveComponent<Script>();
  }

  void Scene::Start(EngineMode mode) {
    OE_ASSERT(initialized, "Starting scene without initialization");
    OE_TRACE("Starting Scene : {}", scene_name);

    /// fix roots first so that scene tree is up to date
    FixRoots();
    /// synchronize before saving state so that captured state is up to date
    Synchronize();

    /// TODO: should the scene be in charge of capturing state like this?
    CaptureScene();

    /// register all entities who dont have a script so they can be accessible to client scripts
    Script& scene_object = scene_entity->GetComponent<Script>();
    for (auto& [id, entity] : entities) {
      scene_object.ApiCall<uint64_t>("RegisterSceneObject", id.Get());
    }

    registry.view<Script>().each([&](Script& script) {
      script.ApiCall("NativeStart");
      script.ApiCall("OnStart");
    });

    OnStart();

    /// rebuild environment after client OnStart/scripts so that
    ///   any modifications get picked up
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

    OnStop();

    RestoreLastCapture();
    /// we have to synchronize here so that scene is same as when it was captured
    ///   (keeps editor state and client state in sync as well as consistent serialization/deserialization)
    Synchronize();
  }

  void Scene::Deactivate() {
    OE_ASSERT(initialized, "Deactivating scene without initialization");
    if (!active) {
      return;
    }

    OnDeactivate();
    active = false;
  }

  void Scene::Synchronize() {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");
    {
      registry.view<RigidBody, Transform>().each([this](RigidBody& body, Transform& transform) {
        OE_ASSERT(body.physics_body != nullptr, "Physics body is null");
        body.physics_body->SetTransform(transform);
        /// TODO: replace this with initial velocity
        body.physics_body->SetVelocity(glm::vec3(0.f));
        body.physics_body->SetAngularVelocity(glm::vec3(0.f));
      });

      registry.view<Collider, Transform>().each([this](Collider& collider, Transform& transform) {
        OE_ASSERT(collider.shape != nullptr, "Collider shape is null");
        collider.shape->SetTransform(transform);
        collider.shape->SetScale(transform.scale);
      });

      physics_world->Simulate(0.00001f);
    }

    RefreshCameraTransforms();
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
      script.ApiCall<float>("EarlyUpdate", std::forward<float>(dt));
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
    OE_ASSERT(initialized, "Updating scene without initialization");
    if (!running) {
      return;
    }

    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }

    /// physics simulations
    if (physics_world != nullptr) {
      physics_world->Simulate(dt * 0.001f);  /// convert ms dt to seconds
    }
    if (physics_world_2d != nullptr) {
      physics_world_2d->Step(dt, 32, 2);
    }

    /// update rigidbody/transform for dynamic bodies
    registry.view<RigidBody, Transform>().each([&](RigidBody& body, Transform& transform) {
      if (physics_world == nullptr) {
        return;
      }
      OE_ASSERT(body.physics_body != nullptr, "Physics body is null");

      if (body.physics_body->GetType() != PhysicsBodyType::DYNAMIC) {
        return;
      }

      if (physics_world->ShouldInterpolateTransform()) {
        Transform new_transform = body.physics_body->InterpolateTransform(physics_world->InterpolationAlpha());
        new_transform.scale = transform.scale;
        transform = new_transform;
      } else {
        Transform new_transform = body.physics_body->GetTransform();
        new_transform.scale = transform.scale;
        transform = new_transform;
      }
    });

    if (physics_world_2d != nullptr) {
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

    /// dynamic physics transform already updated so calculating matrix should be fine
    registry.view<Transform>().each([](Transform& transform) {
      transform.CalcMatrix();

      /// update quaternion
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
      script.ApiCall<float>("Update", std::forward<float>(dt));
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
    OE_ASSERT(initialized, "Updating scene without initialization");

    /// update environment even if scene is not running
    registry.view<LightSource, Transform>().each([&](LightSource& light, Transform& transform) {
      environment->direction_light = std::nullopt;
      environment->point_lights.clear();

      if (light.type == DIRECTION_LIGHT_SRC) {
        transform.erotation = glm::vec3(light.direction_light.direction);
        transform.position = -glm::normalize(glm::vec3(light.direction_light.direction)) * 10.f;
        light.direction_light.position = glm::vec4(transform.position, 1.0);

        float near_plane = 0.1f, far_plane = 100.f;
        glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 light_view = glm::lookAt(transform.position, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        light.direction_light.light_space_matrix = light_projection * light_view;

      } else if (light.type == POINT_LIGHT_SRC) {
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

    registry.view<Camera>().each([&](Camera& camera) {
      if (camera.camera == nullptr) {
        return;
      }

      glm::ivec2 mouse_pos = Mouse::GetPos();

      camera.camera->SetLastMouse(camera.camera->Mouse());
      camera.camera->SetMousePos(mouse_pos);
      camera.camera->SetDeltaMouse({
        camera.camera->Mouse().x - camera.camera->LastMouse().x,
        camera.camera->LastMouse().y - camera.camera->Mouse().y,
      });
    });

    if (!running) {
      return;
    }

    /// prepare scene update
    if (corrupt) {
      Stop();
      return;
    }

    registry.view<Script>().each([&dt](Script& script) {
      script.ApiCall<float>("LateUpdate", std::forward<float>(dt));
    });

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

  // namespace {

  //   template <typename T>
  //     requires requires(const T& t) {
  //       { t.handle } -> std::same_as<AssetHandle>;
  //       { t.material } -> std::same_as<UUID>;
  //     }
  //   void SubmitModel(Ref<SceneRenderer>& renderer, const T& model_comp, const Transform& transform) {
  //     if (!AppState::Assets()->IsValid(model_comp.handle)) {
  //       return;
  //     }

  //     auto model = AssetManager::GetAsset<Model>(model_comp.handle);
  //     renderer->SubmitModel(model, AssetManager::GetMaterialTable(), transform.model_transform, model_comp.material);
  //   }

  // }  // anonymous namespace

  void Scene::Render(Ref<SceneRenderer>& renderer) {
    PROFILE_SECTION("SceneManager--RenderScene");
    {
      PROFILE_SECTION("SceneManager--RenderScene:ClientRender");
      OnRender();
    }

    if (auto primary_cam = GetPrimaryCamera(); primary_cam != nullptr) {
      renderer->SubmitCamera(primary_cam);
    }

    // if (scene_geometry_changed) {
    //   RebuildEnvironment();
    //   scene_geometry_changed = false;
    // }
    renderer->SubmitEnvironment(environment);

    dynamic_mesh_group.each([&renderer](const Mesh& mesh, const Transform& transform) {
      if (!AppState::Assets()->IsValid(mesh.handle)) {
        return;
      }

      auto model = AssetManager::GetAsset<Model>(mesh.handle);
      renderer->SubmitModel(model, AssetManager::GetMaterialTable(), transform.model_transform, mesh.material);
    });

    static_mesh_group.each([&renderer](const StaticMesh& mesh, const Transform& transform) {
      if (!AppState::Assets()->IsValid(mesh.handle)) {
        return;
      }

      auto model = AssetManager::GetAsset<StaticModel>(mesh.handle);
      renderer->SubmitStaticModel(model, AssetManager::GetMaterialTable(), transform.model_transform, mesh.material);
    });
  }

  void Scene::SetDebugPhysicsRendering(bool debug) {
    if (!initialized || physics_world == nullptr) {
      return;
    }

    physics_world->SetDebugRendering(debug);
    physics_world->Simulate(0.00001f);
  }

  bool Scene::IsDebugPhysicsRendering() const {
    if (!initialized || physics_world == nullptr) {
      return false;
    }

    return physics_world->IsDebugRenderEnabled();
  }

  void Scene::RenderPhysicsDebug(Ref<SceneRenderer>& scene_renderer) {
    if (!initialized || physics_world == nullptr) {
      return;
    }
    physics_world->SubmitDebugRender(scene_renderer);
  }

  void Scene::RenderCameraFrustums(Ref<SceneRenderer>& scene_renderer) {
    if (!initialized) {
      return;
    }

    // registry.view<Camera, Transform>().each([&scene_renderer](const Camera& camera, const Transform& transform) {
    //   if (camera.camera == nullptr) {
    //     return;
    //   }

    //   scene_renderer->SubmitDebugDrawCommands(
    //     "Geometry",
    //     {}
    //   );
    // });
  }

  void Scene::RenderLightDebug(Ref<SceneRenderer>& scene_renderer) {
    if (!initialized) {
      return;
    }

    scene_renderer->SubmitDebugDrawCommands(
      "Geometry",
      {
        [&]() {
          AssetHandle cube_handle = ModelFactory::CreateBox();
          Ref<StaticModel> cube_model = AssetManager::GetAsset<StaticModel>(cube_handle);
          OE_ASSERT(cube_model != nullptr, "Cube model is null");
          Ref<ModelSource> model = cube_model->GetModelSource();
          OE_ASSERT(model != nullptr, "Model source is null");

          registry.view<LightSource, Transform>().each([&](const LightSource& light, const Transform& transform) {
            switch (light.type) {
              case DIRECTION_LIGHT_SRC:
                break;

              case POINT_LIGHT_SRC: {
                glm::vec3 color = glm::vec3(light.pointlight.color);
                // bind debug light shader
                model->source_vao->Bind();
                model->source_vao->Draw(DrawMode::TRIANGLES);
                model->source_vao->Unbind();
                break;
              }
              default:
                OE_ASSERT(false, "Unknown light source type!");
            }

            if (light.type == DIRECTION_LIGHT_SRC) {
              // return;
            }

            if (light.type == POINT_LIGHT_SRC) {
              glm::vec3 color = glm::vec3(light.pointlight.color);
              auto model = AssetManager::GetAsset<StaticModel>(cube_handle);
            }
          });
        },
      }
    );
  }

  void Scene::RenderUI() {
    // registry.view<UI>().each([](const UI& ui) {});

    Script& scene_object = scene_entity->GetComponent<Script>();
    scene_object.ApiCall("RenderUI");
  }

  entt::registry& Scene::Registry() {
    return registry;
  }

  Script& Scene::SceneScriptObject() {
    OE_ASSERT(scene_entity != nullptr, "Scene entity is null");
    return scene_entity->GetComponent<Script>();
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

  const bool Scene::IsInitialized() const {
    return initialized;
  }

  const bool Scene::IsActive() const {
    return active;
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
    Entity* ent = entity_allocator.Allocate(registry, id, name);

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

  void Scene::CaptureScene() {
    capture_stack.PushCapture(this);
  }

  void Scene::RestoreLastCapture() {
    capture_stack.PopCapture(this);
  }

  void Scene::ResetPhysicsSimulation() {
    OE_ASSERT(physics_world != nullptr, "Physics world is null");

    physics_world->ResetSimulation(this);

    if (physics_world_2d != nullptr) {
    }
  }

  std::pair<Entity*, Entity*> Scene::BeginContact(UUID entity1, UUID entity2) {
    auto itr1 = entities.find(entity1);
    auto itr2 = entities.find(entity2);
    OE_ASSERT(itr1 != entities.end(), "Entity not found in scene : {}", entity1);
    OE_ASSERT(itr2 != entities.end(), "Entity not found in scene : {}", entity2);

    auto& ent1 = itr1->second;
    auto& ent2 = itr2->second;
    OE_ASSERT(ent1 != nullptr, "Entity is null");
    OE_ASSERT(ent2 != nullptr, "Entity is null");

    if (ent1->HasComponent<Script>()) {
      ent1->GetComponent<Script>().ApiCall<uint64_t>("BeginContact", ent2->GetUUID().Get());
    }

    if (ent2->HasComponent<Script>()) {
      ent2->GetComponent<Script>().ApiCall<uint64_t>("BeginContact", ent1->GetUUID().Get());
    }

    return { ent1, ent2 };
  }

  std::pair<Entity*, Entity*> Scene::ContactPoint(CollisionPointData* point1, CollisionPointData* point2) {
    OE_ASSERT(point1 != nullptr, "Collision point is null");
    OE_ASSERT(point2 != nullptr, "Collision point is null");

    auto itr1 = entities.find(point2->other_entity);
    auto itr2 = entities.find(point1->other_entity);
    OE_ASSERT(itr1 != entities.end(), "Entity not found in scene : {}", point2->other_entity);
    OE_ASSERT(itr2 != entities.end(), "Entity not found in scene : {}", point1->other_entity);

    auto& ent1 = itr1->second;
    auto& ent2 = itr2->second;
    OE_ASSERT(ent1 != nullptr, "Entity is null");
    OE_ASSERT(ent2 != nullptr, "Entity is null");

    if (ent1->HasComponent<Script>()) {
      ent1->GetComponent<Script>().ApiCall<CollisionPointData*>("HandleCollisionPoint", point1);
    }

    if (ent2->HasComponent<Script>()) {
      ent2->GetComponent<Script>().ApiCall<CollisionPointData*>("HandleCollisionPoint", point2);
    }

    return { ent1, ent2 };
  }

  std::pair<Entity*, Entity*> Scene::EndContact(UUID entity1, UUID entity2) {
    auto itr1 = entities.find(entity1);
    auto itr2 = entities.find(entity2);
    OE_ASSERT(itr1 != entities.end(), "Entity not found in scene : {}", entity1);
    OE_ASSERT(itr2 != entities.end(), "Entity not found in scene : {}", entity2);

    auto& ent1 = itr1->second;
    auto& ent2 = itr2->second;
    OE_ASSERT(ent1 != nullptr, "Entity is null");
    OE_ASSERT(ent2 != nullptr, "Entity is null");

    if (ent1->HasComponent<Script>()) {
      ent1->GetComponent<Script>().ApiCall<uint64_t>("EndContact", ent2->GetUUID().Get());
    }

    if (ent2->HasComponent<Script>()) {
      ent2->GetComponent<Script>().ApiCall<uint64_t>("EndContact", ent1->GetUUID().Get());
    }

    return { ent1, ent2 };
  }

  void Scene::RefreshCameraTransforms() {
    /// FIXME: get current viewport size not window size
    auto current_viewport_size = Renderer::WindowSize();
    registry.view<Camera>().each([&](Camera& camera) {
      camera.camera->CalculateMatrix();
      camera.camera->SetViewport(current_viewport_size);
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

  // void Scene::OnAddScript(entt::registry& context, entt::entity entt) {
  //   Entity ent(context, entt);
  //   auto& script = ent.GetComponent<Script>();

  //   auto& tag = ent.GetComponent<Tag>();

  //   auto itr = entities.find(tag.id);
  //   if (itr == entities.end()) {
  //     OE_ERROR("Entity with id [{}] does not exist", tag.id);
  //     return;
  //   }
  //   auto& [id, entity] = *itr;

  //   script.SetHandles();
  // }

  void Scene::OnUpdateTransform(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);
    auto& transform = ent.GetComponent<Transform>();

    if (ent.HasComponent<Mesh>()) {
      auto& mesh = ent.GetComponent<Mesh>();
      Ref<Model> model = AssetManager::GetAsset<Model>(mesh.handle);
      OE_ASSERT(model != nullptr, "Model is null");

      Ref<ModelSource> source = model->GetModelSource();
      OE_ASSERT(source != nullptr, "Model source is null");
    }
  }

  void Scene::OnAddPhysicsObject(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);

    if (!ent.HasComponent<RigidBody>()) {
      ent.AddComponent<RigidBody>();
    }

    if (!ent.HasComponent<Collider>()) {
      ent.AddComponent<Collider>();
    }

    physics_world->CreateBody(ent);
  }

  void Scene::OnDestroyPhysicsObject(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);
    physics_world->DestroyBody(ent);

    OE_ASSERT(ent.HasComponent<RigidBody>(), "RigidBody component still exists");
    OE_ASSERT(ent.HasComponent<Collider>(), "Collider component still exists");

    ent.RemoveComponent<RigidBody>();
    ent.RemoveComponent<Collider>();
  }

  void Scene::OnAddTerrain(entt::registry& context, entt::entity entt) {
    Entity ent(context, entt);
    auto& terrain = ent.GetComponent<Terrain>();

    if (ent.HasComponent<Mesh>()) {
      /// calculate terrain size from mesh vertices

      Terrain::GenerateHeightMap(&ent.GetComponent<Mesh>(), terrain);
    } else {
      /// randomly generate height field
    }
  }

  void Scene::Initialize2DRigidBody(Ref<PhysicsWorld2D>& world, RigidBody2D& body, const Tag& tag, const Transform& transform) {
    body.body_def = b2BodyDef{};
    body.body_def.position.x = transform.position.x;
    body.body_def.position.y = transform.position.y;
    body.body_def.angle = transform.position.z;
    body.body_def.linearDamping = body.linear_drag;
    body.body_def.angularDamping = body.angular_drag;
    body.body_def.gravityScale = body.gravity_scale;
    body.body_def.fixedRotation = body.fixed_rotation;
    body.body_def.bullet = body.bullet;
    body.body_def.userData.pointer = (uintptr_t)tag.id.Get();

    switch (body.type) {
      case STATIC:
        body.body_def.type = b2BodyType::b2_staticBody;
        break;
      case KINEMATIC:
        body.body_def.type = b2BodyType::b2_kinematicBody;
        break;
      case DYNAMIC:
        body.body_def.type = b2BodyType::b2_dynamicBody;
        break;
      default:
        OE_ERROR("Invalid Rigid Body 2D type, can not add to physics scene!");
        return;
    }

    body.physics_body = world->CreateBody(&body.body_def);

    body.mass_data = body.physics_body->GetMassData();
    body.mass_data.mass = body.mass;

    body.physics_body->SetMassData(&body.mass_data);
  }

  void Scene::Initialize2DCollider(Ref<PhysicsWorld2D>& world, RigidBody2D& body, Collider2D& collider, const Transform& transform) {
    b2PolygonShape shape;
    shape.SetAsBox(transform.scale.x * collider.size.x, transform.scale.y * collider.size.y);

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = collider.density;
    fixture_def.friction = collider.friction;

    collider.fixture = body.physics_body->CreateFixture(&fixture_def);
  }

}  // namespace other
