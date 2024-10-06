/**
 * \file script_scene_integration_tests.cpp
 **/
#include "ecs/components/relationship.hpp"
#include "ecs/components/script.hpp"
#include "oetest.hpp"

#include <cstdint>
#include <entt/entity/fwd.hpp>
#include <gtest.h>

#include "mock_app.hpp"

#include "application/app_state.hpp"

#include "ecs/entity.hpp"

#include "scripting/cs/cs_object.hpp"
#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"
#include "scripting/script_object.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace other;

class ScriptSceneIntegrationTests : public OtherTest {
  public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    //// no default behavior for now
    virtual void SetUp() override;
    virtual void TearDown() override;
    
  protected:
    static inline Scope<App> active_app = nullptr;
};

TEST_F(ScriptSceneIntegrationTests , scene_object_generic_functions) {
  Ref<Scene> scene = NewRef<Scene>();
  ASSERT_NE(scene, nullptr);

  ScriptEngine::SetSceneContext(scene);

  scene->Initialize();
  ASSERT_TRUE(scene->IsInitialized());
  
  {
    ScriptRef<CsObject> scene_obj = scene->SceneScriptObject();
    ASSERT_NE(scene_obj , nullptr);
    ASSERT_EQ(scene_obj->GetProperty<void*>("NativeHandle") , (void*)scene.Raw());
  }

  scene->Start();
  scene->EarlyUpdate(0.0f);
  scene->Update(0.0f);
  scene->LateUpdate(0.0f);
  // scene->Render();
  // scene->RenderUI();
  scene->Stop();

  scene->Shutdown();
  ASSERT_TRUE(!scene->IsInitialized());

  scene = nullptr;
}

TEST_F(ScriptSceneIntegrationTests , scene_objects) {

  Ref<Scene> scene = NewRef<Scene>();
  ASSERT_NE(scene, nullptr);
  
  ScriptEngine::SetSceneContext(scene);

  ScriptRef<CsObject> obj = ScriptEngine::GetObjectRef<CsObject>("TestScript" , "Other" , "SandboxScripts");
  ASSERT_NE(obj , nullptr);

  Entity* ent = scene->CreateEntity("TestEntity");
  ASSERT_NE(ent, nullptr);
  ASSERT_TRUE(ent->HasComponent<Tag>());
  ASSERT_TRUE(ent->HasComponent<Transform>());
  ASSERT_TRUE(ent->HasComponent<Relationship>());
   
  Entity* ent2 = scene->CreateEntity("TestEntity2");
  ASSERT_NE(ent2, nullptr);
  ASSERT_TRUE(ent2->HasComponent<Tag>());
  ASSERT_TRUE(ent2->HasComponent<Transform>());
  ASSERT_TRUE(ent2->HasComponent<Relationship>());
  {
    auto& rel = ent->GetComponent<Relationship>();
    rel.children.insert(ent2->GetUUID());

    auto& rel2 = ent2->GetComponent<Relationship>();
    rel2.parent = ent->GetUUID();

    ASSERT_EQ(rel2.parent.value() , ent->GetUUID());
    ASSERT_EQ(rel.children.size() , 1u);
    ASSERT_TRUE(rel.children.contains(ent2->GetUUID()));
  }
  std::cout << fmt::format("Entity 1 : {} | {:p}"sv , ent->GetUUID() , fmt::ptr(ent)) << std::endl;
  std::cout << fmt::format("Entity 2 : {} | {:p}"sv , ent2->GetUUID() , fmt::ptr(ent2)) << std::endl;

  auto& script = ent->AddComponent<Script>();
  script.AddScript("TestScript" , "Other" , "SandboxScripts");
  ASSERT_TRUE(ent->HasComponent<Script>());
  ASSERT_EQ(script.GetScripts().size() , 1u);

  Transform expected_transform;
  {
    auto& transform = ent->GetComponent<Transform>();
    transform.position = glm::vec3(1.1f , 2.2f , 3.3f);
    transform.scale = glm::vec3(3.3f , 2.2f , 1.1f);
    transform.erotation = glm::vec3(1.2f , 3.4f , 5.6f);
    transform.qrotation = glm::quat(transform.erotation);
    expected_transform = transform;
  }
  expected_transform.position += glm::vec3(1.1f , 2.2f , 3.3f);
  expected_transform.scale += glm::vec3(3.3f , 2.2f , 1.1f);
  expected_transform.erotation += glm::vec3(1.2f , 3.4f , 5.6f);
  expected_transform.qrotation = glm::quat(expected_transform.erotation);

  scene->Initialize();
  ASSERT_TRUE(scene->IsInitialized());
  
  {
    ScriptRef<CsObject> scene_obj = scene->SceneScriptObject();
    ASSERT_NE(scene_obj , nullptr);
    ASSERT_EQ(scene_obj->GetProperty<void*>("NativeHandle") , (void*)scene.Raw());
    ASSERT_EQ(scene_obj->GetProperty<uint64_t>("ObjectID") , scene->SceneHandle().Get());
    ASSERT_EQ(scene_obj->GetProperty<uint32_t>("EntityID") , (uint32_t)entt::null);
  }

  {
    bool passed = true;

    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);
  }

  scene->Start();
  
  {
    bool passed = true;
    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);
  }

  scene->EarlyUpdate(0.0f);
  scene->Update(0.0f);
  scene->LateUpdate(0.0f);

  {
    bool passed = true;
    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);

    auto& transform = ent->GetComponent<Transform>();
    ASSERT_TRUE(transform.position == expected_transform.position);
    ASSERT_EQ(transform.scale , expected_transform.scale);
    ASSERT_EQ(transform.erotation , expected_transform.erotation);
    ASSERT_EQ(transform.qrotation , expected_transform.qrotation);
  }

  // scene->Render();
  // scene->RenderUI();
  scene->Stop();

  scene->Shutdown();
  ASSERT_TRUE(!scene->IsInitialized());

  scene = nullptr;
}

TEST_F(ScriptSceneIntegrationTests , scene_editing_simulating_tests) {
  Ref<Scene> scene = NewRef<Scene>();
  ASSERT_NE(scene, nullptr);

  ScriptEngine::SetSceneContext(scene);

  Entity* ent = scene->CreateEntity("TestEntity");
  ASSERT_NE(ent, nullptr);
   
  Entity* ent2 = scene->CreateEntity("TestEntity2");
  ASSERT_NE(ent2, nullptr);
  ASSERT_TRUE(ent2->HasComponent<Tag>());
  ASSERT_TRUE(ent2->HasComponent<Transform>());
  ASSERT_TRUE(ent2->HasComponent<Relationship>());
  {
    auto& rel = ent->GetComponent<Relationship>();
    rel.children.insert(ent2->GetUUID());

    auto& rel2 = ent2->GetComponent<Relationship>();
    rel2.parent = ent->GetUUID();

    ASSERT_EQ(rel2.parent.value() , ent->GetUUID());
    ASSERT_EQ(rel.children.size() , 1u);
    ASSERT_TRUE(rel.children.contains(ent2->GetUUID()));
  }
  std::cout << fmt::format("Entity 1 : {} | {:p}"sv , ent->GetUUID() , fmt::ptr(ent)) << std::endl;
  std::cout << fmt::format("Entity 2 : {} | {:p}"sv , ent2->GetUUID() , fmt::ptr(ent2)) << std::endl;

  constexpr std::string_view kTestScript = "TestScript";
  constexpr std::string_view kTestScript2 = "TestScript2";
  constexpr std::string_view kTestScript3 = "TestScript3";

  auto add_script_to_object = [](Script& script , const std::string_view& name) {
    ScriptRef<CsObject> obj = ScriptEngine::GetObjectRef<CsObject>(name , "Other" , "SandboxScripts");
    ASSERT_NE(obj , nullptr);

    script.AddScript(name , "Other" , "SandboxScripts");
  };

  auto remove_script_from_object = [](Script& script , const std::string_view& name) {
    script.RemoveScript(name);
  };

  {
    auto& script = ent->AddComponent<Script>();
    add_script_to_object(script , kTestScript);
    add_script_to_object(script , kTestScript2);
    add_script_to_object(script , kTestScript3);
  }

  auto& transform = ent->GetComponent<Transform>();
  transform.position = glm::vec3(1.1f , 2.2f , 3.3f);
  transform.scale = glm::vec3(3.3f , 2.2f , 1.1f);
  transform.erotation = glm::vec3(1.2f , 3.4f , 5.6f);
  transform.qrotation = glm::quat(transform.erotation);

  scene->Initialize();
  ASSERT_TRUE(scene->IsInitialized());
  {
    ScriptRef<CsObject> scene_obj = scene->SceneScriptObject();
    ASSERT_NE(scene_obj , nullptr);
    ASSERT_EQ(scene_obj->GetProperty<void*>("NativeHandle") , (void*)scene.Raw());
  
    bool passed = true;

    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);
  }

  scene->Start();
  {
    bool passed = true;

    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);
  }
  scene->EarlyUpdate(0.0f);
  scene->Update(0.0f);
  scene->LateUpdate(0.0f);
  scene->Stop();

  {
    auto& script = ent->GetComponent<Script>();
    remove_script_from_object(script , kTestScript3);
    remove_script_from_object(script , kTestScript2);

    auto& script2 = ent2->AddComponent<Script>();
    add_script_to_object(script2 , kTestScript2);
  }
  scene->Start();
  {
    bool passed = true;

    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);
  }
  scene->EarlyUpdate(0.0f);
  scene->Update(0.0f);
  scene->LateUpdate(0.0f);
  scene->Stop();

  {
    auto& script = ent->GetComponent<Script>();
    remove_script_from_object(script , kTestScript);
    add_script_to_object(script, kTestScript3);
  }
  scene->Start();
  {
    bool passed = true;

    auto& registry = scene->Registry();
    registry.view<Script>().each([&](Script& script) {
      passed &= script.ValidateScripts();
    });
    ASSERT_TRUE(passed);
  }
  scene->EarlyUpdate(0.0f);
  scene->Update(0.0f);
  scene->LateUpdate(0.0f);
  scene->Stop();

  {
    auto& script = ent->GetComponent<Script>();
    remove_script_from_object(script , kTestScript3);

    auto& script2 = ent2->GetComponent<Script>();
    remove_script_from_object(script2 , kTestScript2);
  }

  scene->Shutdown();
  ASSERT_TRUE(!scene->IsInitialized());

  scene = nullptr;
}

void ScriptSceneIntegrationTests::SetUpTestSuite() {
  ConfigTable test_config = ConfigTable{};
  test_config.Add("log", "console-level" , "debug" , true);
  test_config.Add("log", "file-level" , "trace" , true);
  test_config.Add("log", "path" , "logs/script-scene-integration-test.log" , true);
  test_config.Add("project" , "script-bin-dir" , "SandboxScripts" , true);
  test_config.Add("project" , "assets-dir" , "./tests/scripts" , true);
  test_config.Add("script-engine.C#" , "modules" , std::vector{ "SandboxScripts.dll"s } , true);
  
  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Scene Integration Test Main Thread");
  
  active_app = NewScope<TestApp>(cmdline , test_config);
  active_app->Load();
  AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                      active_app->asset_handler , active_app->project_metadata);
  
  ScriptEngine::Initialize(test_config);
}

void ScriptSceneIntegrationTests::TearDownTestSuite() {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::Shutdown()); 
  ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  active_app = nullptr;
  CloseLog();
}

void ScriptSceneIntegrationTests::SetUp() { 
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));
}

void ScriptSceneIntegrationTests::TearDown() {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}