/**
 * \file script_scene_integration_tests.cpp
 **/
#include "oetest.hpp"

#include "mock_app.hpp"

#include "application/app_state.hpp"
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
    virtual void SetUp() override {}
    virtual void TearDown() override {}
    
  protected:
    static Scope<App> active_app;
};

Scope<App> ScriptSceneIntegrationTests::active_app = nullptr;

TEST_F(ScriptSceneIntegrationTests , scene_object) {
  Ref<Scene> scene = NewRef<Scene>();
  scene->Initialize();
}

void ScriptSceneIntegrationTests::SetUpTestSuite() {
  StashConfig();
  
  config = ConfigTable{};
  config->Add("log", "console-level" , "info" , true);
  config->Add("log", "file-level" , "trace" , true);
  config->Add("log", "path" , "logs/script-scene-integration-test.log" , true);
  config->Add("project" , "script-bin-dir" , "TestScripts" , true);
  config->Add("project" , "assets-dir" , "./tests/scripts" , true);
  config->Add("script-engine.C#" , "modules" , std::vector{ "TestScripts.dll"s } , true);
  OpenLog();
  
  active_app = NewScope<TestApp>(cmdline , *config);
  active_app->Load();
  AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                      active_app->asset_handler , active_app->project_metadata);
  
  ScriptEngine::Initialize(*config);
  /// no python 
  ASSERT_EQ(ScriptEngine::GetModules().size() , 2);
  ASSERT_EQ(ScriptEngine::GetModules().at(LanguageModuleType::CS_MODULE).module->GetModules().size() , 1);
  ASSERT_EQ(ScriptEngine::GetModules().at(LanguageModuleType::LUA_MODULE).module->GetModules().size() , 0);
  ASSERT_EQ(ScriptEngine::GetModules().at(LanguageModuleType::LUA_MODULE).module->GetModules().size() , 0);
}

void ScriptSceneIntegrationTests::TearDownTestSuite() {
  ScriptEngine::Shutdown(); 
  active_app = nullptr;
  CloseLog();

  ApplyStashedConfig();
}