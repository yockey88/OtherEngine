/**
 * \file script_engine_tests.cpp
 **/
#include "oetest.hpp"

#include <gtest.h>
#include <string>

#include "mock_app.hpp"

#include "core/defines.hpp"
#include "application/app_state.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"
#include "scripting/script_object.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace other;

class ScriptEngineTests : public OtherTest {
  public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    //// no default behavior for now
    virtual void SetUp() override {}
    virtual void TearDown() override {}
    
  protected:
    static Scope<App> active_app;
};

Scope<App> ScriptEngineTests::active_app = nullptr;

TEST_F(ScriptEngineTests , load_project_modules) {  
#if 0 // toggle to disable test
  GTEST_SKIP();
#endif

  /// no python module 
  ASSERT_EQ(ScriptEngine::GetModules().size() , 2);
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));
  
  for (auto& [id , module] : ScriptEngine::GetModules()) {
    println(" > {}"sv , module.module->GetModuleName());
    for (auto& [id , script] : module.module->GetModules()) {
      println("   - {}"sv , script->ModuleName());
    }
  }

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}

TEST_F(ScriptEngineTests , dynamic_add_project_modules) {
#if 0 // toggle to disable test
  GTEST_SKIP();
#endif

  /// CsCore and no lua scripts
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));

  ScriptEngine::GetModule(CS_MODULE)->LoadScriptModule({
    .name = "OtherEngine.CsCore" ,
    .path = "./bin/Debug/OtherEngine-CsCore/net8.0/OtherEngine-CsCore.dll" ,
  });
  ASSERT_TRUE(CheckNumScripts(1 , 0 , 0));

  ScriptEngine::GetModule(CS_MODULE)->LoadScriptModule({
    .name = "SandboxScripts" ,
    .path = "./bin/Debug/SandboxScripts/net8.0/SandboxScripts.dll" ,
  });
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));

  ScriptEngine::GetModule(CS_MODULE)->UnloadScript("SandboxScripts");
  ASSERT_TRUE(CheckNumScripts(1 , 0 , 0));

  ScriptEngine::GetModule(LUA_MODULE)->LoadScriptModule({
    .name = "SandboxLua" ,
    .path = "./tests/scripts/lua/engine_script1.lua" ,
  });
  ASSERT_TRUE(CheckNumScripts(1 , 1 , 0));

  ScriptEngine::GetModule(CS_MODULE)->UnloadScript("OtherEngine.CsCore");
  ASSERT_TRUE(CheckNumScripts(0 , 1 , 0));

  ScriptEngine::GetModule(LUA_MODULE)->UnloadScript("SandboxLua");
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}

TEST_F(ScriptEngineTests , script_object) {
#if 0 // toggle to disable test
  GTEST_SKIP();
#endif

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));

  Ref<ScriptModule> sandbox = ScriptEngine::GetModule(CS_MODULE)->GetScriptModule("SandboxScripts");
  ASSERT_NE(sandbox , nullptr);

  Ref<CsObject> obj = sandbox->GetScriptObject<CsObject>("TestScript" , "Other");
  ASSERT_NE(obj , nullptr);

  obj->CallMethod<void>("Method");

  ScriptEngine::GetModule(LUA_MODULE)->LoadScriptModule({
    .name = "SandboxLua" ,
    .path = "./tests/scripts/lua/engine_script1.lua" ,
  });
  ASSERT_TRUE(CheckNumScripts(2 , 1 , 0));

  Ref<ScriptModule> lua_sandbox = ScriptEngine::GetModule(LUA_MODULE)->GetScriptModule("SandboxLua");
  ASSERT_NE(lua_sandbox , nullptr);

  Ref<LuaObject> lua_obj = lua_sandbox->GetScriptObject<LuaObject>("TestScript");
  ASSERT_NE(lua_obj , nullptr);

  lua_obj->CallMethod<void>("Method");
  auto res = lua_obj->CallMethod<std::string>("test");
  ASSERT_EQ(res , "TestScript.test()");

  Ref<LuaObject> lua_obj2 = lua_sandbox->GetScriptObject<LuaObject>("TestScript2");
  ASSERT_NE(lua_obj2 , nullptr);

  auto res2 = lua_obj2->CallMethod<std::string>("test");
  ASSERT_EQ(res2 , "TestScript2.test()");
  
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::GetModule(LUA_MODULE)->UnloadScript("SandboxLua"));
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}

TEST_F(ScriptEngineTests , retrieve_core_objects) {
#if 0 // toggle to disable test
  GTEST_SKIP();
#endif

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));

  Ref<ScriptObject> scene_obj = nullptr;
  ASSERT_NO_FATAL_FAILURE(scene_obj = ScriptEngine::GetScriptObject("Scene" , "Other" , "OtherEngine.CsCore"));
  ASSERT_NE(scene_obj , nullptr);

  scene_obj = nullptr;

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}

TEST_F(ScriptEngineTests , get_object_method) {
#if 0 // toggle to disable test
  GTEST_SKIP();
#endif

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::LoadProjectModules());
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));

  ScriptRef<CsObject> cs_obj = ScriptEngine::GetObjectRef<CsObject>("Scene" , "Other", "OtherEngine.CsCore");
  ASSERT_NE(cs_obj , nullptr);

  ASSERT_NO_FATAL_FAILURE(cs_obj->Initialize());
  ASSERT_NO_FATAL_FAILURE(cs_obj->Start());
  ASSERT_NO_FATAL_FAILURE(cs_obj->EarlyUpdate(0.16f));
  ASSERT_NO_FATAL_FAILURE(cs_obj->Update(0.16f));
  ASSERT_NO_FATAL_FAILURE(cs_obj->LateUpdate(0.16f));
  ASSERT_NO_FATAL_FAILURE(cs_obj->Render());
  ASSERT_NO_FATAL_FAILURE(cs_obj->RenderUI());
  ASSERT_NO_FATAL_FAILURE(cs_obj->Stop());
  ASSERT_NO_FATAL_FAILURE(cs_obj->Shutdown());
  cs_obj = nullptr;

  ScriptEngine::GetModule(LUA_MODULE)->LoadScriptModule({
    .name = "SandboxLua" ,
    .path = "./tests/scripts/lua/engine_script1.lua" ,
  });
  ASSERT_TRUE(CheckNumScripts(2 , 1 , 0));

  ScriptRef<LuaObject> lua_obj = ScriptEngine::GetObjectRef<LuaObject>("TestScript" , "" , "SandboxLua");
  ASSERT_NE(lua_obj , nullptr);

  ASSERT_NO_FATAL_FAILURE(lua_obj->Initialize());
  ASSERT_NO_FATAL_FAILURE(lua_obj->Start());
  ASSERT_NO_FATAL_FAILURE(lua_obj->EarlyUpdate(0.16f));
  ASSERT_NO_FATAL_FAILURE(lua_obj->Update(0.16f));
  ASSERT_NO_FATAL_FAILURE(lua_obj->LateUpdate(0.16f));
  ASSERT_NO_FATAL_FAILURE(lua_obj->Render());
  ASSERT_NO_FATAL_FAILURE(lua_obj->RenderUI());
  ASSERT_NO_FATAL_FAILURE(lua_obj->Stop());
  ASSERT_NO_FATAL_FAILURE(lua_obj->Shutdown());
  lua_obj = nullptr;

  lua_obj = ScriptEngine::GetObjectRef<LuaObject>("TestScript2" , "" , "SandboxLua");
  ASSERT_NE(lua_obj , nullptr);

  ASSERT_NO_FATAL_FAILURE(lua_obj->Initialize());
  ASSERT_NO_FATAL_FAILURE(lua_obj->Start());
  ASSERT_NO_FATAL_FAILURE(lua_obj->EarlyUpdate(0.16f));
  ASSERT_NO_FATAL_FAILURE(lua_obj->Update(0.16f));
  ASSERT_NO_FATAL_FAILURE(lua_obj->LateUpdate(0.16f));
  ASSERT_NO_FATAL_FAILURE(lua_obj->Render());
  ASSERT_NO_FATAL_FAILURE(lua_obj->RenderUI());
  ASSERT_NO_FATAL_FAILURE(lua_obj->Stop());
  ASSERT_NO_FATAL_FAILURE(lua_obj->Shutdown());
  lua_obj = nullptr;

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::GetModule(LUA_MODULE)->UnloadScript("SandboxLua"));
  ASSERT_TRUE(CheckNumScripts(2 , 0 , 0));

  ASSERT_NO_FATAL_FAILURE(ScriptEngine::UnloadProjectModules());
  ASSERT_TRUE(CheckNumScripts(0 , 0 , 0));
}

void ScriptEngineTests::SetUpTestSuite() {
  ConfigTable test_config;
  test_config.Add("log", "console-level" , "debug" , true);
  test_config.Add("log", "file-level" , "trace" , true);
  test_config.Add("log", "path" , "logs/script-engine-test.log" , true);
  test_config.Add("project" , "script-bin-dir" , "SandboxScripts" , true);
  test_config.Add("project" , "assets-dir" , "./tests/scripts" , true);
  test_config.Add("script-engine.C#" , "modules" , std::vector{ "SandboxScripts.dll"s } , true);
  Logger::Open(test_config);
  Logger::Instance()->RegisterThread("Script Engine Test Main Thread");
  
  active_app = NewScope<TestApp>(cmdline , test_config);
  active_app->Load();
  AppState::Initialize(active_app.get() , active_app->layer_stack , active_app->scene_manager , 
                      active_app->asset_handler , active_app->project_metadata);
  
  ScriptEngine::Initialize(test_config);
}

void ScriptEngineTests::TearDownTestSuite() {
  ASSERT_NO_FATAL_FAILURE(ScriptEngine::Shutdown()); 
  active_app = nullptr;
  ASSERT_NO_FATAL_FAILURE(AppState::Shutdown());
  ASSERT_NO_FATAL_FAILURE(CloseLog());
}
