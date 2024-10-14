/**
 * \file scene_tree_tests.cpp
 **/
#include <cstdint>

#include <gtest.h>

#include "core/config.hpp"
#include "core/ref_counted.hpp"

#include "application/app_state.hpp"

#include "ecs/components/transform.hpp"
#include "ecs/entity.hpp"
#include "scene/scene_tree.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"

#include "mock_app.hpp"
#include "oetest.hpp"

using namespace std::string_view_literals;

using other::NewRef;
using other::NewScope;
using other::Ref;
using other::Scope;

class SceneTreeTests : public other::OtherTest {
 public:
  static void SetUpTestSuite();
  static void TearDownTestSuite();

  //// no default behavior for now
  virtual void SetUp() override;
  virtual void TearDown() override;

 protected:
  static inline Scope<other::App> active_app = nullptr;
};

using other::Entity;
using other::Scene;
using other::SceneTree;
using other::Transform;

using other::BvhTree;
using other::Octant;
using other::Octree;

using other::ConfigTable;

using other::AppState;
using other::Logger;

TEST_F(SceneTreeTests, add_scene) {
  Ref<SceneTree> tree = NewRef<SceneTree>();
  ASSERT_NE(tree, nullptr);
  ASSERT_NE(tree->octree, nullptr);
  ASSERT_EQ(tree->octree->NumNodes(), 1);
  ASSERT_EQ(tree->octree->Depth(), 0);
  ASSERT_TRUE(tree->octree->GetSpace().IsLeaf());

  glm::vec3 max = { 0.5f, 0.5f, 0.5f };

  Ref<Scene> scene = NewRef<Scene>();
  ASSERT_NE(scene, nullptr);

  Entity* ent1 = scene->CreateEntity("Test1");
  ASSERT_NE(ent1, nullptr);
  {
    auto& transform = ent1->GetComponent<Transform>();
    transform.position = max;
  }

  Entity* ent2 = scene->CreateEntity("Test2");
  ASSERT_NE(ent2, nullptr);
  {
    auto& t = ent2->GetComponent<Transform>();
    t.position = { -1 * max.x, -1 * max.y, -1 * max.z };
  }

  Entity* ent3 = scene->CreateEntity("Test3");
  ASSERT_NE(ent3, nullptr);
  {
    auto& t = ent3->GetComponent<Transform>();
    t.position = { -1 * max.x, max.y, -1 * max.z };
  }

  Entity* ent4 = scene->CreateEntity("Test4");
  ASSERT_NE(ent4, nullptr);
  {
    auto& t = ent4->GetComponent<Transform>();
    t.position = { max.x, -1 * max.y, max.z };
  }

  tree->AddScene(scene, glm::zero<glm::vec3>());
  ASSERT_EQ(tree->octree->NumNodes(), 1u);

  tree->octree->PrintNodes(std::cout);
  ASSERT_EQ(tree->octree->NumNodes(), 1);
  ASSERT_EQ(tree->octree->Depth(), 0);
  ASSERT_TRUE(tree->octree->GetSpace().IsLeaf());
  ASSERT_EQ(tree->octree->GetSpace().entities.size(), 4u);

  Octant& space = tree->octree->GetSpace();
  ASSERT_EQ(space.GetMaxDepth(), 0);
  ASSERT_TRUE(space.IsLeaf());
  ASSERT_EQ(space.partition_location, 0b000);
  ASSERT_EQ(space.entities.size(), 4);
  ASSERT_EQ(space.Min(), glm::vec3(-0.5f));
  ASSERT_EQ(space.Max(), glm::vec3(0.5f));

  tree->bvh->PrintNodes(std::cout);
}

TEST_F(SceneTreeTests, add_scene_high_res) {
  Ref<SceneTree> tree = NewRef<SceneTree>();
  ASSERT_NE(tree, nullptr);
  ASSERT_NE(tree->octree, nullptr);

  tree->octree->Subdivide({ 1.f, 1.f, 1.f }, 3);
  ASSERT_EQ(tree->octree->NumNodes(), 1 + 8 + 64 + 512);  // 8^0 + 8^1 + 8^2 + 8^3
  ASSERT_EQ(tree->octree->Depth(), 3);
  ASSERT_FALSE(tree->octree->GetSpace().IsLeaf());

  //   glm::vec3 max = {0.5f, 0.5f, 0.5f};

  //   Ref<Scene> scene = NewRef<Scene>();
  //   ASSERT_NE(scene, nullptr);

  //   Entity* ent1 = scene->CreateEntity("Test1");
  //   ASSERT_NE(ent1, nullptr);
  //   {
  //     auto& transform = ent1->GetComponent<Transform>();
  //     transform.position = max;
  //   }

  //   Entity* ent2 = scene->CreateEntity("Test2");
  //   ASSERT_NE(ent2, nullptr);
  //   {
  //     auto& t = ent2->GetComponent<Transform>();
  //     t.position = {-1 * max.x, -1 * max.y, -1 * max.z};
  //   }

  //   Entity* ent3 = scene->CreateEntity("Test3");
  //   ASSERT_NE(ent3, nullptr);
  //   {
  //     auto& t = ent3->GetComponent<Transform>();
  //     t.position = {-1 * max.x, max.y, -1 * max.z};
  //   }

  //   Entity* ent4 = scene->CreateEntity("Test4");
  //   ASSERT_NE(ent4, nullptr);
  //   {
  //     auto& t = ent4->GetComponent<Transform>();
  //     t.position = {max.x, -1 * max.y, max.z};
  //   }

  //   tree->AddScene(scene, glm::zero<glm::vec3>());
  //   ASSERT_EQ(tree->octree->NumNodes(), 1 + 8 + 64 + 512);
  //   ASSERT_EQ(tree->octree->Depth(), 3);
  //   ASSERT_FALSE(tree->octree->GetSpace().IsLeaf());
  //   ASSERT_EQ(tree->octree->GetSpace().entities.size(), 4);

  //   Octant& posxyz = tree->octree->FindNode(0b000, 3);
  //   ASSERT_EQ(posxyz.GetMaxDepth(), 0);
  //   ASSERT_TRUE(posxyz.IsLeaf());
  //   ASSERT_EQ(posxyz.partition_location, 0b000);
  //   ASSERT_EQ(posxyz.entities.size(), 1);

  //   Octant& negxyz = tree->octree->FindNode(0b111, 3);
  //   ASSERT_EQ(negxyz.GetMaxDepth(), 0);
  //   ASSERT_TRUE(negxyz.IsLeaf());
  //   ASSERT_EQ(negxyz.partition_location, 0b111);
  //   ASSERT_EQ(negxyz.entities.size(), 1);

  //   Octant& posxznegy = tree->octree->FindNode(0b101, 3);
  //   ASSERT_EQ(posxznegy.GetMaxDepth(), 0);
  //   ASSERT_TRUE(posxznegy.IsLeaf());
  //   ASSERT_EQ(posxznegy.partition_location, 0b101);
  //   ASSERT_EQ(posxznegy.entities.size(), 1);

  //   Octant& negyposxz = tree->octree->FindNode(0b010, 3);
  //   ASSERT_EQ(negyposxz.GetMaxDepth(), 0);
  //   ASSERT_TRUE(negyposxz.IsLeaf());
  //   ASSERT_EQ(negyposxz.partition_location, 0b010);
  //   ASSERT_EQ(negyposxz.entities.size(), 1);
}

void SceneTreeTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level", "trace", true);
  test_config.Add("log", "file-level", "trace", true);
  test_config.Add("log", "path", "logs/scene-tree-test.log", true);

  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Scene Integration Test Main Thread");

  active_app = NewScope<TestApp>(cmdline, test_config);
  active_app->Load();
  ASSERT_NO_FATAL_FAILURE(AppState::Initialize(active_app.get(), active_app->layer_stack, active_app->scene_manager,
                                               active_app->asset_handler, active_app->project_metadata));
}

void SceneTreeTests::TearDownTestSuite() {
  ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  active_app = nullptr;
  CloseLog();
}

void SceneTreeTests::SetUp() {
}

void SceneTreeTests::TearDown() {
}
