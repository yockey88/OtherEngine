/**
 * \file scene_tree_tests.cpp
 **/
#include "oetest.hpp"

#include "core/ref_counted.hpp"
#include "application/app_state.hpp"

#include "ecs/entity.hpp"

#include "scene/octree.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"

#include "mock_app.hpp"
#include <gtest.h>

using namespace other;
using namespace std::string_view_literals;

class SceneTreeTests : public OtherTest {
  public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    //// no default behavior for now
    virtual void SetUp() override;
    virtual void TearDown() override;
    
  protected:
    static inline Scope<App> active_app = nullptr;
};

class SceneTree : public RefCounted {
  public:
    SceneTree() {
      octree = NewRef<Octree>();
    }
    ~SceneTree() {}

    void AddScene(Ref<Scene> scene , const glm::vec3& position) {
      OE_ASSERT(scene != nullptr , "Scene is null!");
      OE_DEBUG("Adding scene to octree at <{}>" , position);

      octree->AddScene(scene , position);
    }
      
    Ref<Octree> octree = nullptr;

  private:

    std::vector<Ref<Scene>> scenes;
};

TEST_F(SceneTreeTests , add_scene) {
  Ref<SceneTree> tree = NewRef<SceneTree>();
  ASSERT_NE(tree , nullptr);
  ASSERT_NE(tree->octree , nullptr);
  ASSERT_EQ(tree->octree->NumOctants() , 1);
  ASSERT_EQ(tree->octree->Depth() , 0);
  ASSERT_TRUE(tree->octree->GetSpace().IsLeaf());
  tree->octree->PrintOctants(std::cout);
  
  glm::vec3 max = { 0.5f , 0.5f , 0.5f };

  Ref<Scene> scene = NewRef<Scene>();
  ASSERT_NE(scene , nullptr);

  Entity* ent1 = scene->CreateEntity("Test1");
  ASSERT_NE(ent1 , nullptr);
  {
    auto& transform = ent1->GetComponent<Transform>();
    transform.position = max;
  }

  Entity* ent2 = scene->CreateEntity("Test2");
  ASSERT_NE(ent2 , nullptr);
  {
    auto& t = ent2->GetComponent<Transform>();
    t.position = { -1 * max.x , -1 * max.y , -1 * max.z };
  }

  Entity* ent3 = scene->CreateEntity("Test3");
  ASSERT_NE(ent3 , nullptr);
  {
    auto& t = ent3->GetComponent<Transform>();
    t.position = { -1 * max.x , max.y , -1 * max.z };
  }

  Entity* ent4 = scene->CreateEntity("Test4"); 
  ASSERT_NE(ent4 , nullptr);
  {
    auto& t = ent4->GetComponent<Transform>();
    t.position = { max.x , -1 * max.y , max.z };
  }

  tree->AddScene(scene , glm::zero<glm::vec3>());
  ASSERT_EQ(tree->octree->NumOctants() , 1 + 8);
  ASSERT_EQ(tree->octree->Depth() , 1);
  ASSERT_FALSE(tree->octree->GetSpace().IsLeaf());
  tree->octree->PrintOctants(std::cout);

  glm::vec3 id{ 1.f };
  Octant& space = tree->octree->GetSpace();
  ASSERT_EQ(space.tree_index , 0);
  ASSERT_EQ(space.depth , 1);
  
  {
    Octant& space = tree->octree->FindOctant(0b111 , 0);  
    ASSERT_EQ(space.IsLeaf() , false);
    ASSERT_EQ(space.tree_index , 0);
    ASSERT_EQ(space.depth , 1);
  }

  {
    Octant& space = tree->octree->FindOctant(0b000 , 0);  
    ASSERT_EQ(space.IsLeaf() , false);
    ASSERT_EQ(space.tree_index , 0);
    ASSERT_EQ(space.depth , 1);
  }

  {
    Octant& posxyz = tree->octree->FindOctant(0b000 , 1);
    ASSERT_TRUE(posxyz.IsLeaf());
    ASSERT_EQ(posxyz.tree_index , 1 + kLocationIndex.at(0b000));
    ASSERT_EQ(posxyz.entities.size() , 1u);
  }

  {
    Octant& negxyz = tree->octree->FindOctant(0b111 , 1);
    ASSERT_EQ(negxyz.tree_index , 1 + kLocationIndex.at(0b111));
    ASSERT_EQ(negxyz.entities.size() , 1u);
  }

  {
    Octant& negxposy = tree->octree->FindOctant(0b101 , 1);
    ASSERT_EQ(negxposy.tree_index , 1 + kLocationIndex.at(0b101));
    ASSERT_EQ(negxposy.entities.size() , 1u);
  }

  {
    Octant& posxnegy = tree->octree->FindOctant(0b010 , 1);
    ASSERT_EQ(posxnegy.tree_index , 1 + kLocationIndex.at(0b010));
    ASSERT_EQ(posxnegy.entities.size() , 1u);
  }
}

void SceneTreeTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level" , "trace" , true);
  test_config.Add("log", "file-level" , "trace" , true);
  test_config.Add("log", "path" , "logs/scene-tree-test.log" , true);
  // test_config.Add("project" , "script-bin-dir" , "SandboxScripts" , true);
  // test_config.Add("project" , "assets-dir" , "./tests/scripts" , true);
  // test_config.Add("script-engine.C#" , "modules" , std::vector{ "SandboxScripts.dll"s } , true);
  
  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Scene Integration Test Main Thread");
  
  active_app = NewScope<TestApp>(cmdline , test_config);
  active_app->Load();
  AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                      active_app->asset_handler , active_app->project_metadata);
  
  ScriptEngine::Initialize(test_config);
}

void SceneTreeTests::TearDownTestSuite() {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::Shutdown()); 
  ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  active_app = nullptr;
  CloseLog();
}

void SceneTreeTests::SetUp() {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(1 , 0 , 0));
}

void SceneTreeTests::TearDown() {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}
