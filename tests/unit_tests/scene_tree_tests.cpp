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
    SceneTree() {}
    ~SceneTree() {
      octree = nullptr;
    }

    void AddScene(Ref<Scene> scene , const glm::vec3& position) {
      if (octree == nullptr) {
        octree = NewRef<Octree>();
      }

      const auto& entities = scene->SceneEntities();
      // for (const auto& [id , ent] : entities) {
      //   octree->AddEntity(ent);
      // }
    }

    Ref<Octree> octree = nullptr;

  private:
    std::vector<Ref<Scene>> scenes;
};

TEST_F(SceneTreeTests , attach_scene) {
  SceneTree tree;
  Ref<Scene> scene = NewRef<Scene>();
  ASSERT_NE(scene , nullptr);

  Entity* ent1 = scene->CreateEntity("Test1");
  // Entity* ent2 = scene->CreateEntity("Test2");
  // Entity* ent3 = scene->CreateEntity("Test3");
  // Entity* ent4 = scene->CreateEntity("Test4");
  ASSERT_NE(ent1 , nullptr);
  // ASSERT_NE(ent2 , nullptr);
  // ASSERT_NE(ent3 , nullptr);
  // ASSERT_NE(ent4 , nullptr);

  glm::vec3 max = { 0.5f , 0.5f , 0.5f };
  {
    auto& transform = ent1->GetComponent<Transform>();
    transform.position = max;
  }
  
  // {
  //   auto& t = ent2->GetComponent<Transform>();
  //   t.position = { -1 * max.x , -1 * max.y , -1 * max.z };
  // }
  // 
  // {
  //   auto& t = ent3->GetComponent<Transform>();
  //   t.position = { -1 * max.x , max.y , -1 * max.z };
  // }

  // {
  //   auto& t = ent3->GetComponent<Transform>();
  //   t.position = { max.x , -1 * max.y , max.z };
  // }

  tree.AddScene(scene , { 0.f , 0.f , 0.f });
  // tree.octree->PrintOctants(std::cout);
}

void SceneTreeTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level" , "debug" , true);
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
