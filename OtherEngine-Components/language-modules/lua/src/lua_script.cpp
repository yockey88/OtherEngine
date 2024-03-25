/**
 * \file lua_script.cpp
 **/
#include "lua_script.hpp"

extern "C" {
  #include <lua/lua.h>
  #include <lua/lualib.h>
  #include <lua/lauxlib.h>
}

namespace other {

  LuaScript::LuaScript(Engine* engine , const std::string& path) 
    : ScriptModule(engine) , path(path) {
  }

  LuaScript::~LuaScript() {
  }

  void LuaScript::Initialize() {
    std::filesystem::path p(path);
    for (const auto& entry : std::filesystem::directory_iterator(p)) {
      if (entry.is_regular_file() && entry.path().extension() == ".lua") {
        LogDebug("Found script: {}", entry.path().string());
        script_paths.push_back(std::filesystem::absolute(entry.path()).string());
      }
    }
  }

  void LuaScript::Shutdown() {
    for (auto& [id , script] : loaded_objects) {
      script.Shutdown();
    }
    loaded_objects.clear();
  }

  ScriptObject* LuaScript::GetScript(const std::string& name , const std::string& nspace) {
    if (script_paths.empty()) {
      LogError("No scripts found in path: {}", path);
      return nullptr;
    }

    auto hash = FNV(name);
    if (loaded_objects.find(hash) != loaded_objects.end()) {
      return &loaded_objects[hash];
    }

    auto path_name = name + ".lua";
    auto itr = std::find_if(script_paths.begin() , script_paths.end() , [&path_name](const std::string& path) {
      return path.find(path_name) != std::string::npos;
    });
    if (itr != script_paths.end()) {
      loaded_objects[hash] = LuaObject(name , *itr);
      loaded_objects[hash].InitializeScriptMethods();
      return &loaded_objects[hash];
    }

    LogError("Script not found: {}", path_name);
    return nullptr;
  }

} // namespace other
