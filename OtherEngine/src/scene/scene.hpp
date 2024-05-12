/**
 * \file scene/scene.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_HPP
#define OTHER_ENGINE_SCENE_HPP

#include <map>
#include <filesystem>

#include <entt/entt.hpp>

// #include "scene/octree.hpp"
#include "core/config.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"

namespace other {

  class Entity;

  class Scene : public RefCounted {
    public:
      Scene(const Path& scenepath);
      ~Scene();

      void Start(); 
      void Stop(); 

      const Path& GetPath() const { return scene_path; }
      const std::map<UUID , Entity*>& SceneEntities() const;

    protected:
      virtual void OnStart() {}
      virtual void OnStop() {}

    private:
      friend class Entity;
      // Octree space_partition;

      ConfigTable scene_table;

      bool corrupt = true;

      Path scene_path;
      entt::registry registry;

      std::map<UUID , Entity*> entities{};

      void LoadScene();
      
      void AddEntity(const std::string& name); 
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_HPP
