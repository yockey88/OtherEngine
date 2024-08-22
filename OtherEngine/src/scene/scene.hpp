/**
 * \file scene/scene.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_HPP
#define OTHER_ENGINE_SCENE_HPP

#include <entt/entity/fwd.hpp>
#include <map>

#include <entt/entt.hpp>

#include "core/uuid.hpp"
#include "core/ref.hpp"
#include "asset/asset.hpp"

#include "scene/octree.hpp"
#include "scene/environment.hpp"

#include "scripting/script_object.hpp"

#include "physics/2D/physics_world_2d.hpp"
#include "physics/3D/physics_world.hpp"

#include "rendering/scene_renderer.hpp"

namespace other {

  class Entity;

  class Scene : public Asset {
    public:
      OE_ASSET(SCENE);

      Scene();
      ~Scene();

      void Initialize();
      void Start(); 

      void EarlyUpdate(float dt);
      void Update(float dt);
      void LateUpdate(float dt);

      Ref<CameraBase> GetPrimaryCamera() const;

      void Render(Ref<SceneRenderer> scene_renderer);
      void RenderUI();

      void Stop(); 
      void Shutdown();

      entt::registry& Registry();

      ScriptObject* SceneScriptObject();

      Ref<PhysicsWorld2D> Get2DPhysicsWorld() const;
      Ref<PhysicsWorld> GetPhysicsWorld() const;

      Ref<Environment> GetEnvironment() const;

      const bool IsInitialized() const;
      const bool IsRunning() const;
      const bool IsDirty() const;

      bool EntityExists(UUID id) const;
      bool EntityExists(const std::string& name) const;

      size_t NumCameras() const;

      const std::map<UUID , Entity*>& RootEntities() const;
      const std::map<UUID , Entity*>& SceneEntities() const;

      bool HasEntity(const std::string& name) const;
      bool HasEntity(UUID id) const;

      Entity* GetEntity(const std::string& name);
      Entity* GetEntity(UUID id) const;

      Entity* CreateEntity(const std::string& name = "");
      Entity* CreateEntity(const std::string& name , UUID id);

      void DestroyEntity(UUID id);

      void RenameEntity(UUID curr_id , UUID new_id , const std::string_view name);

      void ParentEntity(UUID id , UUID parent_id);
      void OrphanEntity(UUID id);

    protected:
      other::AssetHandle model_handle;
      Ref<StaticModel> model = nullptr;
      Ref<ModelSource> model_source = nullptr;

      Ref<Environment> environment = nullptr;

      void OnAddRigidBody2D(entt::registry& context , entt::entity ent);
      void OnAddCollider2D(entt::registry& context , entt::entity ent);
      
      void OnAddRigidBody(entt::registry& context , entt::entity ent);
      void OnAddCollider(entt::registry& context , entt::entity ent);

      void OnAddLightSource(entt::registry& context , entt::entity ent);
      void OnRemoveLightSource(entt::registry& context , entt::entity ent);

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
      // Octree space_partition;

      bool initialized = false;
      bool running = false;
      bool corrupt = false;

      entt::registry registry;

      ScriptObject* scene_object = nullptr;

      Ref<PhysicsWorld2D> physics_world_2d;
      Ref<PhysicsWorld> physics_world;

      std::map<UUID , Entity*> root_entities{};
      std::map<UUID , Entity*> entities{};

      void FixRoots();
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_HPP
