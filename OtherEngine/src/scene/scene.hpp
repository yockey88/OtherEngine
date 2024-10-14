/**
 * \file scene/scene.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_HPP
#define OTHER_ENGINE_SCENE_HPP

#include <map>

#include <entt/entt.hpp>

#include <core/dotother_defines.hpp>
#include <hosting/native_object.hpp>
#include <reflection/echo_defines.hpp>
#include <reflection/object_proxy.hpp>
#include <reflection/reflected_object.hpp>

#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "asset/asset.hpp"

#include "ecs/component.hpp"
#include "ecs/components/light_source.hpp"
#include "ecs/components/mesh.hpp"
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "ecs/components/transform.hpp"
#include "scene/environment.hpp"

#include "physics/2D/physics_world_2d.hpp"
#include "physics/3D/physics_world.hpp"
#include "rendering/scene_renderer.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/script_object.hpp"

namespace echo = dotother::echo;

namespace other {

  class Entity;

  class Scene : public Asset, dotother::NObject {
    ECHO_REFLECT();

   public:
    OE_ASSET(SCENE);

    Scene();
    virtual ~Scene() override;

    UUID SceneHandle() const;

    template <typename Fn>
    void ForEachEntity(Fn&& fn) {
      for (auto& [id, ent] : entities) {
        fn(ent);
      }
    }

    void Initialize();
    void Start(EngineMode mode = EngineMode::DEBUG);

    void EarlyUpdate(float dt);
    void Update(float dt);
    void LateUpdate(float dt);

    Ref<CameraBase> GetPrimaryCamera() const;

    void Render(Ref<SceneRenderer>& scene_renderer);

    void RenderUI();

    void Stop();
    void Shutdown();

    bool IsHandleValid(Entity* ent) const;

    entt::registry& Registry();

    ScriptRef<CsObject> SceneScriptObject();

    Ref<PhysicsWorld2D> Get2DPhysicsWorld() const;
    Ref<PhysicsWorld> GetPhysicsWorld() const;

    Ref<Environment> GetEnvironment() const;

    const bool IsInitialized() const;
    const bool IsRunning() const;
    const bool IsDirty() const;

    bool EntityExists(UUID id) const;
    bool EntityExists(const std::string& name) const;

    size_t NumCameras() const;

    const std::map<UUID, Entity*>& RootEntities() const;
    const std::map<UUID, Entity*>& SceneEntities() const;

    bool HasEntity(const std::string& name) const;
    bool HasEntity(UUID id) const;

    Entity* GetEntity(const std::string& name);
    Entity* GetEntity(UUID id) const;

    Entity* CreateEntity(const std::string& name = "");
    Entity* CreateEntity(const std::string& name, UUID id);

    void DestroyEntity(UUID id);

    void RenameEntity(UUID curr_id, UUID new_id, const std::string_view name);

    void ParentEntity(UUID id, UUID parent_id);
    void OrphanEntity(UUID id);

    void GeometryChanged();
    void RebuildEnvironment();

   protected:
    other::AssetHandle model_handle;
    Ref<StaticModel> model = nullptr;
    Ref<ModelSource> model_source = nullptr;

    Ref<Environment> environment = nullptr;

    void RenderToPipeline(const std::string_view plname, Ref<SceneRenderer>& scene_renderer, bool do_debug = false);

    void OnAddRigidBody2D(entt::registry& context, entt::entity ent);
    void OnAddCollider2D(entt::registry& context, entt::entity ent);

    void OnAddRigidBody(entt::registry& context, entt::entity ent);
    void OnAddCollider(entt::registry& context, entt::entity ent);

    void RefreshCameraTransforms();

    virtual void OnInit() {}
    virtual void OnStart() {}

    virtual void OnEarlyUpdate(float dt) {}
    virtual void OnUpdate(float dt) {}
    virtual void OnLateUpdate(float dt) {}

    virtual void OnRender() {}
    virtual void OnRenderUI() {}

    virtual void OnStop() {}
    virtual void OnShutdown() {}

   private:
    friend class Entity;
    friend class SceneSerializer;

    bool initialized = false;
    bool running = false;
    bool corrupt = false;

    /// true until first render
    bool scene_geometry_changed = true;

    entt::registry registry;
    UUID scene_handle;
    ScriptRef<CsObject> scene_object = nullptr;

    SystemGroup<Relationship> connection_group;
    SystemGroup<LightSource, Transform> light_group;
    SystemGroup<Script> script_group;

    template <RenderableComp RC>
    using RenderGroup = SystemGroup<RC, Transform>;

    RenderGroup<Mesh> dynamic_mesh_group;
    RenderGroup<StaticMesh> static_mesh_group;

    Ref<PhysicsWorld2D> physics_world_2d;
    Ref<PhysicsWorld> physics_world;

    std::map<UUID, Entity*> root_entities{};
    std::map<UUID, Entity*> entities{};

    template <ComponentType T1, ComponentType T2 = NullComponent, ComponentType T3 = NullComponent>
    auto GetGroup() -> SystemGroup<T1, T2, T3> {
      return registry.group<T1>(entt::get<T2>, entt::exclude<T3>);
    }

    void FixRoots();
    void BuildGroups();
  };

}  // namespace other

ECHO_TYPE(
  type(other::Scene, refl::attr::bases<dotother::NObject>),
  func(GetEntity));

#endif  // !OTHER_ENGINE_SCENE_HPP
