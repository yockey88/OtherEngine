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
    if (Path{ path }.extension() != ".lua" || !Filesystem::FileExists(path)) {
      OE_ERROR("Failed to load lua script {}" , path);
      corrupt = true;
      return;
    }

    if (corrupt) {
      return;
    }

    try {
      sol::function_result res = lua_state.safe_script_file(path);
      if (!res.valid()) {
        OE_ERROR("Failed to load lua script file {}" , path);
        return;
      }
    } catch (const std::exception& e) {
      OE_ERROR("Failed to load lua script file {}" , path);
      OE_ERROR("  > Error = {}" , e.what());
      return;
    }

    SetPath(path);
    valid = true;
  }

  void LuaScript::Shutdown() {
    for (auto& [id , obj] : loaded_objects) {
      obj.Shutdown();
    }
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
  
    auto& obj = loaded_objects[id] = LuaObject(module_name , name , &lua_state);
    obj.InitializeScriptMethods();
    obj.InitializeScriptFields();

    return &loaded_objects[id];
  }

} // namespace other
