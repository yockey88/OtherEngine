/**
 * \file scene/scene.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_HPP
#define OTHER_ENGINE_SCENE_HPP

#include <map>

#include <entt/entt.hpp>

#include "core/ref_counted.hpp"
#include "core/uuid.hpp"
#include "scene/octree.hpp"
#include "rendering/render_batch.hpp"

namespace other {

  class Entity;

  class Scene : public RefCounted {
    public:
      Scene();
      ~Scene();

      void Initialize();
      void Start(); 
      void Update(float dt);
      void Render();
      void RenderUI();
      void Stop(); 
      void Shutdown();

      void Refresh();

      const bool IsInitialized() const;
      const bool IsRunning() const;
      const bool IsDirty() const;

      bool EntityExists(UUID id) const;
      bool EntityExists(const std::string& name) const;

      std::vector<BatchData> GetRenderBatchData() const;

      const std::map<UUID , Entity*>& RootEntities() const;
      const std::map<UUID , Entity*>& SceneEntities() const;

      Entity* GetEntity(UUID id) const;

      Entity* CreateEntity(const std::string& name);
      Entity* CreateEntity(const std::string& name , UUID id);

    protected:
      virtual void OnInit() {}
      virtual void OnStart() {}
      virtual void OnUpdate(float dt) {}
      virtual void OnRender() {}
      virtual void OnRenderUI() {}
      virtual void OnStop() {}
      virtual void OnShutdown() {}

    private:
      friend class Entity;
      // Octree space_partition;

      bool initialized = false;
      bool running = false;
      bool corrupt = false;

      entt::registry registry;

      std::map<UUID , Entity*> root_entities{};
      std::map<UUID , Entity*> entities{};
      
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_HPP
