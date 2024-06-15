/**
 * \file scripting/lua/lua_script.cpp
 **/
#include "scripting/lua/lua_script.hpp"

#include "core/filesystem.hpp"
#include <sol/forward.hpp>
#include <sol/load_result.hpp>

namespace other {

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

    for (const auto& p : paths) {
      try {
        sol::function_result res = lua_state.safe_script_file(p);
        if (!res.valid()) {
          OE_ERROR("Failed to load lua script file {}" , p);
          continue;
        }
      } catch (const std::exception& e) {
        OE_ERROR("Failed to load lua script file {}" , p);
        OE_ERROR("  > Error = {}" , e.what());
        continue;
      }
    }

    SetPaths(paths);
    valid = true;

    if (reloaded) {
      reloaded = false;
    }
  }

  void LuaScript::Shutdown() {
    loaded_objects.clear();
  }

  void LuaScript::Reload() {
    /// no need to build anything here
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
    loaded_objects[id].InitializeScriptFields();

    return &loaded_objects[id];
  }

} // namespace other
