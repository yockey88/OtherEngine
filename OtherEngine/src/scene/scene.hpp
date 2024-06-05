/**
 * \file scene/scene.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_HPP
#define OTHER_ENGINE_SCENE_HPP

#include <map>

#include <entt/entt.hpp>

#include "scene/octree.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

namespace other {

  class Entity;

  class Scene : public RefCounted {
    public:
      Scene();
      ~Scene();

      void Initialize();
      void Start(); 
      void Update(float dt);
      void Stop(); 
      void Shutdown();

      const bool IsInitialized() const;
      const bool IsRunning() const;
      const bool IsDirty() const;

      const std::map<UUID , Entity*>& SceneEntities() const;

      Entity* GetEntity(UUID id) const;

      Entity* CreateEntity(const std::string& name);
      Entity* CreateEntity(const std::string& name , UUID id);

    protected:
      virtual void OnInit() {}
      virtual void OnStart() {}
      virtual void OnUpdate(float dt) {}
      virtual void OnStop() {}
      virtual void OnShutdown() {}

    private:
      friend class Entity;
      // Octree space_partition;

      bool initialized = false;
      bool running = false;
      bool corrupt = true;

      entt::registry registry;

      std::map<UUID , Entity*> entities{};
      
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_HPP
