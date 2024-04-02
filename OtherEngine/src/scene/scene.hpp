/**
 * \file scene/scene.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_HPP
#define OTHER_ENGINE_SCENE_HPP

#include <entt/entt.hpp>

// #include "scene/octree.hpp"

namespace other {

  class Scene {
    public:
      Scene() {}
      ~Scene() {}

      void AddEntity(const std::string& name); 

    private:
      // Octree space_partition;

      entt::registry registry;
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_HPP
