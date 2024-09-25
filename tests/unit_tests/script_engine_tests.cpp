/**
 * \file script_engine_tests.cpp
 **/
#include "oetest.hpp"

#include <gtest.h>

#include "mock_app.hpp"

#include "application/app_state.hpp"
#include "scripting/script_engine.hpp"

using namespace std::string_literals;
class ScriptEngineTests : public other::OtherTest {
  public:
    virtual void SetUp() override {
      other::OtherTest::SetUp();

      config.Add("script-engine.C#" , "modules" , std::vector{ "SandboxScripts"s } , true);
      
      active_app = other::NewScope<TestApp>(cmdline , config);
      active_app->Load();
      other::AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                          active_app->asset_handler , active_app->project_metadata);
    }

    virtual void TearDown() override {
      active_app = nullptr;

      other::OtherTest::TearDown();
    }

  protected:
    other::Scope<other::App> active_app = nullptr;
};

TEST_F(ScriptEngineTests , attempt_load) {
  ASSERT_NO_FATAL_FAILURE(other::ScriptEngine::Initialize(config));
  /// C# and LUA
  ASSERT_EQ(other::ScriptEngine::GetModules().size() , 2);

  ASSERT_NO_FATAL_FAILURE(other::ScriptEngine::LoadProjectModules());
  
  auto& modules = other::ScriptEngine::GetModules();
  for (auto& module : modules) {
    auto& scripts = module.second.module->GetModules();

    if (module.second.module->GetLanguageType() == other::LanguageModuleType::CS_MODULE) {
      ASSERT_EQ(scripts.size() , 1);
    } else if (module.second.module->GetLanguageType() == other::LanguageModuleType::LUA_MODULE) {
      ASSERT_EQ(scripts.size() , 0);
    }
  }

  ASSERT_NO_FATAL_FAILURE(other::ScriptEngine::Shutdown());
}