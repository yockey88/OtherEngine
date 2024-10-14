/**
 * \file scene/scene_tree.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_TREE_HPP
#define OTHER_ENGINE_SCENE_TREE_HPP

#ifdef OE_TESTING_ENVIRONMENT
#include <gtest/gtest.h>
#endif  // !OE_TESTING_ENVIRONMENT

#include "core/ref_counted.hpp"

#include "scene/bvh.hpp"
#include "scene/octree.hpp"

namespace other {

  class SceneTree : public RefCounted {
   public:
    SceneTree()
        : global_position(glm::zero<glm::vec3>()), bvh(NewRef<BvhTree>(glm::vec3{ 0.f, 0.f, 0.f })),
          octree(NewRef<Octree>(glm::vec3{ 0.f, 0.f, 0.f })) {}

    void AddScene(Ref<Scene>& scene, const glm::vec3& position);

    glm::vec3 global_position = glm::zero<glm::vec3>();

    /// for partitioning entities
    Ref<BvhTree> bvh = nullptr;
    /// for partitioning space
    Ref<Octree> octree = nullptr;

   private:
    std::map<UUID, Ref<Scene>> scenes;

#ifdef OE_TESTING_ENVIRONMENT
    friend class OtherTest;
    FRIEND_TEST(SceneTreeTests, add_scene);
#endif
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_TREE_HPP
