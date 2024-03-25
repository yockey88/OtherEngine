/**
 * \file lua_module.cpp
 **/
#include "lua_module.hpp"

extern "C" {
  #include <lua/lua.h>
  #include <lua/lualib.h>
  #include <lua/lauxlib.h>
}

#include "lua_script.hpp"

namespace other {

  LuaModule::LuaModule(Engine* engine) 
    : LanguageModule(engine) {
  }

  LuaModule::~LuaModule() {
  }

  void LuaModule::Initialize() {
    LogDebug("Initializing Lua Module");

    LoadScriptModule(ScriptModuleInfo {
      .name = "Lua-Core" ,
      .paths = {
        "./OtherEngine-Components/language-modules/lua/core" 
      } ,
    });
  }

  void LuaModule::Reinitialize() {
    LogDebug("Reinitializing Lua Module");

    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
    }

    for (auto& [id , module] : loaded_modules) {
      module->Initialize();
    }
  }

  void LuaModule::Shutdown() {
    LogDebug("Shutting down Lua Module");

    auto it = loaded_modules.begin();
    while (it != loaded_modules.end()) {
      it->second->Shutdown();
      it = loaded_modules.erase(it);
    }

    LogDebug("Lua Module Shutdown");
  }

  ScriptModule* LuaModule::GetScriptModule(const std::string& name) {
    UUID id = FNV(name);
    auto* module = GetScriptModule(id);
    if (module == nullptr) {
      LogError("Script module {} not found" , name);
    }

    return module;
  }

  ScriptModule* LuaModule::GetScriptModule(const UUID& id) {
    if (loaded_modules.find(id) != loaded_modules.end()) {
      return loaded_modules[id];
    }

    LogError("Script module {} not found" , id);
    return nullptr;
  }

  ScriptModule* LuaModule::LoadScriptModule(const ScriptModuleInfo& module_info) {
    UUID id = FNV(module_info.name);
    if (loaded_modules.find(id) != loaded_modules.end()) {
      LogError("Attempting to load script module {} that is already loaded" , module_info.name);
      return nullptr;
    }

    LogDebug("Loading script module {} from {}" , module_info.name , module_info.paths[0]);

    loaded_modules[id] = new LuaScript(GetEngine() , module_info.paths[0]);
    loaded_modules[id]->Initialize();
    return loaded_modules[id];
  }

  void LuaModule::UnloadScriptModule(const std::string& name) {
    UUID id = FNV(name);
    if (loaded_modules.find(id) == loaded_modules.end()) {
      LogError("Attempting to unload script module {} that is not loaded" , name);
      return;
    }

    loaded_modules[id]->Shutdown();
    delete loaded_modules[id];
    loaded_modules.erase(id);
  }

  std::string_view LuaModule::GetModuleName() const {
    return "Lua";
  }

  std::string_view LuaModule::GetModuleVersion() const {
    return "0.0.1";
  }

} // namespace other

OE_API other::Plugin* CreatePlugin(other::Engine* engine) {
  return new other::LuaModule(engine);
}

OE_API void DestroyPlugin(other::Plugin* plugin) {
  delete plugin;
}
