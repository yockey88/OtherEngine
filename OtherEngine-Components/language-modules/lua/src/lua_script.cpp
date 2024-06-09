/**
 * \file lua_script.cpp
 **/
#include "lua_script.hpp"

#include "core/filesystem.hpp"

namespace other {

  LuaScript::LuaScript(Engine* engine , const std::vector<std::string>& paths) 
      : ScriptModule(engine) , paths(paths) {
  }

  LuaScript::~LuaScript() {
  }

  void LuaScript::Initialize() {
    bool corrupt = false;
    for (const auto& file : paths) {
      if (Path{ file }.extension() != ".lua" || !Filesystem::FileExists(file)) {
        OE_ERROR("Failed to load lua script {}" , file);
        corrupt = true;
        continue;
      }
    }

    if (corrupt) {
      return;
    }
    
    try {
      lua_state.open_libraries(sol::lib::base , sol::lib::io , sol::lib::os , sol::lib::ffi , 
                               sol::lib::jit  , sol::lib::math , sol::lib::utf8 , sol::lib::bit32 , 
                               sol::lib::count , sol::lib::table , sol::lib::string , sol::lib::debug ,
                               sol::lib::package , sol::lib::coroutine);

      /// load scripts 
      bool load_failed = false;
      for (const auto& p : paths) {
        sol::function_result res = lua_state.safe_script_file(p);
        if (!res.valid()) {
          OE_ERROR("Failed to load lua script {}" , p);
          load_failed = true;
        }
      }

      if (load_failed) {
        lua_state = sol::state();
        return;
      }

      /// temporary script binding methods
      lua_state["Other"]["LogTrace"] = [&](const std::string& str) {
        println("[ Lua Trace ]> {}" , str);
      };
      lua_state["Other"]["OE_DEBUG"] = [&](const std::string& str) {
        println("[ Lua Debug ]> {}" , str);
      };
      lua_state["Other"]["OE_INFO"] = [&](const std::string& str) {
        println("[ Lua Info ]> {}" , str);
      };
      lua_state["Other"]["LogWarn"] = [&](const std::string& str) {
        println("[ Lua Warning ]> {}" , str);
      };
      lua_state["Other"]["OE_ERROR"] = [&](const std::string& str) {
        println("[ Lua Error ]> {}" , str);
      };
      lua_state["Other"]["LogCritical"] = [&](const std::string& str) {
        println("[ Lua Critical ]> {}" , str);
      };

    } catch (const std::exception& e) {
      OE_ERROR("Exception caught loading lua scripts : {}" , e.what());
      return;
    }

    valid = true;
  }

  void LuaScript::Shutdown() {
    for (auto& [id , script] : loaded_objects) {
      script.Shutdown();
    }
    loaded_objects.clear();

    lua_state.collect_garbage();
    lua_state = sol::state();
  }

  ScriptObject* LuaScript::GetScript(const std::string& name , const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }
    
    UUID id = FNV(name);
    if (loaded_objects.find(id) != loaded_objects.end()) {
      LuaObject* obj = &loaded_objects[id];
      if (obj != nullptr) {
        return obj;
      }
    }

    OE_INFO("Lua object {} loaded" , name);
  
    loaded_objects[id] = LuaObject(name , &lua_state);
    loaded_objects[id].InitializeScriptMethods();

    return &loaded_objects[id];
  }

} // namespace other
