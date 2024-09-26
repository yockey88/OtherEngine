/**
 * \file script_engine_tests.cpp
 **/
#include "oetest.hpp"

#include <gtest.h>

#include "mock_app.hpp"

#include "application/app_state.hpp"
#include "scripting/script_engine.hpp"

using namespace std::string_literals;
using namespace other;

class ScriptEngineTests : public OtherTest {
  public:
    virtual void SetUp() override {
      config.Add("log", "console-level" , "debug" , true);
      config.Add("log", "file-level" , "trace" , true);
      config.Add("log", "path" , "logs/script-engine-test.log" , true);
      OtherTest::SetUp();

      config.Add("project" , "script-bin-dir" , "SandboxScripts" , true);
      config.Add("script-engine.C#" , "modules" , std::vector{ "SandboxScripts.dll"s } , true);
      
      active_app = NewScope<TestApp>(cmdline , config);
      active_app->Load();
      AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                          active_app->asset_handler , active_app->project_metadata);
    }

    virtual void TearDown() override {
      active_app = nullptr;
      OtherTest::TearDown();
    }

  protected:
    Scope<App> active_app = nullptr;
};

bool CheckNumScripts(uint32_t cs , uint32_t lua) {
  auto& modules = ScriptEngine::GetModules();
  for (auto& module : modules) {
    auto& scripts = module.second.module->GetModules();

    switch (module.second.module->GetLanguageType()) {
      case LanguageModuleType::CS_MODULE:
        return scripts.size() == cs;
      case LanguageModuleType::LUA_MODULE:
        return scripts.size() == lua;
      default:
        return false;
    }
  }
}

TEST_F(ScriptEngineTests , attempt_load) {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::Initialize(config));
  /// C# and LUA
  ASSERT_EQ(ScriptEngine::GetModules().size() , 2);
  ASSERT_TRUE(CheckNumScripts(1 , 0));

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  // ASSERT_TRUE(CheckNumScripts(2 , 0));


  // ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::Shutdown());
}