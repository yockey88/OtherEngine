/**
 * \file script_engine_tests.cpp
 **/
#include "oetest.hpp"

#include <gtest.h>
#include <string>

#include "mock_app.hpp"

#include "application/app_state.hpp"
#include "physics/3D/activation_listener.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/script_engine.hpp"
#include "scripting/script_object.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace other;

class ScriptEngineTests : public OtherTest {
  public:
    static void SetUpTestSuite() {
      StashConfig();
      
      config = ConfigTable{};
      config->Add("log", "console-level" , "debug" , true);
      config->Add("log", "file-level" , "trace" , true);
      config->Add("log", "path" , "logs/script-engine-test.log" , true);
      config->Add("project" , "script-bin-dir" , "SandboxScripts" , true);
      config->Add("project" , "assets-dir" , "./tests/scripts" , true);
      config->Add("script-engine.C#" , "modules" , std::vector{ "SandboxScripts.dll"s } , true);
      OpenLog();
      
      active_app = NewScope<TestApp>(cmdline , *config);
      active_app->Load();
      AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                          active_app->asset_handler , active_app->project_metadata);
      
      ScriptEngine::Initialize(*config);
    }

    //// no default behavior for now
    virtual void SetUp() override {}
    virtual void TearDown() override {}
    
    static void TearDownTestSuite() {
      ScriptEngine::Shutdown(); 
      active_app = nullptr;
      CloseLog();

      ApplyStashedConfig();
    }

  protected:
    static Scope<App> active_app;
};

Scope<App> ScriptEngineTests::active_app = nullptr;

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
  return false;
}

TEST_F(ScriptEngineTests , load_project_modules) {
  ASSERT_EQ(ScriptEngine::GetModules().size() , 2);
  ASSERT_TRUE(CheckNumScripts(1 , 0));

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0));

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
}

TEST_F(ScriptEngineTests , dynamic_add_project_modules) {
  /// CsCore and no lua scripts
  ASSERT_TRUE(CheckNumScripts(1 , 0));

  ScriptEngine::GetModule(CS_MODULE)->LoadScriptModule({
    .name = "SandboxScripts" ,
    .paths = { "./bin/Debug/SandboxScripts/net8.0/SandboxScripts.dll" } ,
  });

  ASSERT_TRUE(CheckNumScripts(2 , 0));

  ScriptEngine::GetModule(CS_MODULE)->UnloadScript("SandboxScripts");
  ASSERT_TRUE(CheckNumScripts(1 , 0));
}

TEST_F(ScriptEngineTests , script_object) {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0));

  ScriptModule* sandbox = ScriptEngine::GetModule(CS_MODULE)->GetScriptModule("SandboxScripts");
  ASSERT_NE(sandbox , nullptr);

  Ref<other::CsObject> obj = sandbox->GetScriptObject<other::CsObject>("TestScript" , "Other");
  ASSERT_NE(obj , nullptr);

  obj->CallMethod<void>("Method");

  ScriptEngine::GetModule(LUA_MODULE)->LoadScriptModule({
    .name = "SandboxLua" ,
    .paths = { "engine_script1.lua" } ,
  });
  ASSERT_TRUE(CheckNumScripts(2 , 1));

  ScriptModule* lua_sandbox = ScriptEngine::GetModule(LUA_MODULE)->GetScriptModule("SandboxLua");
  ASSERT_NE(lua_sandbox , nullptr);

  Ref<other::LuaObject> lua_obj = lua_sandbox->GetScriptObject<other::LuaObject>("TestScript" , "Other");
  ASSERT_NE(lua_obj , nullptr);

  auto res = lua_obj->CallMethod<std::string>("test");
  ASSERT_EQ(res , "TestScript.test()");

  lua_obj->CallMethod<void>("Method");

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
}