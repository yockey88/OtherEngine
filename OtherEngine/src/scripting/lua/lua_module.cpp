/**
 * \file scripting/lua/lua_module.cpp
 **/
#include "scripting/lua/lua_module.hpp"

#include "core/filesystem.hpp"
#include "scripting/lua/lua_error_handlers.hpp"
#include "scripting/lua/lua_script.hpp"
#include "scripting/lua/lua_bindings.hpp"
#include "scripting/lua/lua_math_bindings.hpp"

namespace other {

  bool LuaModule::Initialize() {
    OE_DEBUG("Initializing Lua Module");

    try {
      lua_state.open_libraries(sol::lib::base  , sol::lib::io   , 
                               sol::lib::os    , sol::lib::math ,
                               sol::lib::table , sol::lib::string , 
                               sol::lib::debug , sol::lib::package , 
                               sol::lib::coroutine);
      lua_state.set_exception_handler(LuaExceptionHandler);

      lua_script_bindings::BindGlmTypes(lua_state);
      lua_script_bindings::BindCoreTypes(lua_state);

    } catch (const std::exception& e) {
      OE_ERROR("Exception caught loading lua scripts : {}" , e.what());
      return false;
    }

    load_success = true;
    return load_success;
  }

  void LuaModule::Shutdown() {
    lua_state.collect_garbage();
    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      delete module;
    }
    loaded_modules.clear();

    lua_state = sol::state();

    OE_DEBUG("Lua Module Shutdown");
  }
  
  void LuaModule::Reload() {
    Shutdown();
    Initialize();

    for (const auto& [id , data] : loaded_modules_data) {
      LoadScriptModule(data);
    }

    load_success = true;
  }

  ScriptModule* LuaModule::GetScriptModule(const std::string& name) {
    UUID id = FNV(name);
    auto* module = GetScriptModule(id);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
    }

    return module;
  }

  ScriptModule* LuaModule::GetScriptModule(const UUID& id) {
    if (loaded_modules.find(id) != loaded_modules.end()) {
      return loaded_modules[id];
    }

    OE_ERROR("Script module {} not found" , id);
    return nullptr;
  }

  ScriptModule* LuaModule::LoadScriptModule(const ScriptModuleInfo& module_info) {
    if (module_info.paths.size() < 1) {
      OE_ERROR("Attempting to create lua script from empty module data");
      return nullptr;
    }

    std::string case_insensitive_name;
    std::transform(module_info.name.begin() , module_info.name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Loading Lua script module {} [{}]" , module_info.name , id);

    if (loaded_modules.find(id) != loaded_modules.end()) {
      OE_ERROR("Attempting to load script module {} that is already loaded" , module_info.name);
      return nullptr;
    }

    if (!Filesystem::PathExists(module_info.paths[0])) {
      OE_ERROR("Script module {} path {} does not exist" , module_info.name , module_info.paths[0]);
      return nullptr;
    }

    loaded_modules[id] = new LuaScript(lua_state , module_info.paths[0]);
    loaded_modules[id]->Initialize();
    
    loaded_modules_data[id] = module_info;

    return loaded_modules[id];
  }

  void LuaModule::UnloadScriptModule(const std::string& name) {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Unloading C# script module {} [{}]" , name , id);

    if (loaded_modules.find(id) == loaded_modules.end()) {
      OE_ERROR("Attempting to unload script module {} that is not loaded" , name);
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
