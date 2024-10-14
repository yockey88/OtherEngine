/**
 * \file scene/scene_tree.cpp
 **/
#include "scene/scene_tree.hpp"

namespace other {

  void SceneTree::AddScene(Ref<Scene>& scene , const glm::vec3& position) {
    UUID scene_handle = scene->SceneHandle();
    auto [_ , res] = scenes.insert({ scene_handle , scene });
    if (!res) {
      OE_WARN("Scene [{}] already in scene-tree!" , scene_handle);
      return;
    }

    bvh->AddScene(scene , position);
    octree->AddScene(scene , position);
  } 

} // namespace other
