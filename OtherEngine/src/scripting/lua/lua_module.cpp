/**
 * \file scripting/lua/lua_module.cpp
 **/
#include "scripting/lua/lua_module.hpp"

#include <filesystem>

#include "core/filesystem.hpp"
#include "scripting/lua/lua_script.hpp"
#include "scripting/lua/lua_bindings.hpp"


namespace other {

  bool LuaModule::Initialize() {
    OE_DEBUG("Initializing Lua Module");

    Path lua_core = Filesystem::GetEngineCoreDir().string() / Path{ kLuaCorePath };

    if (!Filesystem::PathExists(lua_core)) {
      OE_ERROR("Failed to find lua scripting core");
      return false;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(lua_core)) {
      if (entry.is_regular_file()) {
       
        auto path = entry.path();
        if (path.extension() == ".lua") {
          OE_DEBUG("Adding lua core file {}" , path.string());
          core_files.push_back(path.string());
        }
      }
    }
    
    try {
      lua_state.open_libraries(sol::lib::base , sol::lib::io , sol::lib::os , sol::lib::math ,
                               sol::lib::table , sol::lib::string , sol::lib::debug , sol::lib::package , 
                               sol::lib::coroutine);

      /// load scripts 
      bool load_failed = false;
      for (const auto& p : core_files) {
        sol::function_result res = lua_state.safe_script_file(p);
        if (!res.valid()) {
          OE_ERROR("Failed to load lua script {}" , p);
          load_failed = true;
        }
      }

      if (load_failed) {
        lua_state = sol::state();
        return false;
      }

      LuaScriptBindings::InitializeBindings(lua_state);

    } catch (const std::exception& e) {
      OE_ERROR("Exception caught loading lua scripts : {}" , e.what());
      return false;
    }

    load_success = true;
    return load_success;
  }

  void LuaModule::Shutdown() {
    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
      delete module;
    }
    loaded_modules.clear();

    OE_DEBUG("Lua Module Shutdown");
  }
  
  void LuaModule::Reload() {
    lua_state.collect_garbage();
    loaded_modules.clear();

    lua_state = sol::state();

    try {
      lua_state.open_libraries(sol::lib::base , sol::lib::io , sol::lib::os , sol::lib::math ,
                               sol::lib::table , sol::lib::string , sol::lib::debug , sol::lib::package , 
                               sol::lib::coroutine);

      /// load scripts 
      bool load_failed = false;
      for (const auto& p : core_files) {
        sol::function_result res = lua_state.safe_script_file(p);
        if (!res.valid()) {
          OE_ERROR("Failed to load lua script {}" , p);
          load_failed = true;
        }
      }

      if (load_failed) {
        OE_ERROR("Reloaded lua state failed, lua scripts corrupt");
        return;
      }

      LuaScriptBindings::InitializeBindings(lua_state);

    } catch (const std::exception& e) {
      OE_ERROR("Exception caught re-loading lua scripts : {}" , e.what());
      return;
    }

    for (auto& [id , mod] : loaded_modules) {
      mod->Reload();
      delete mod;
    }
    loaded_modules.clear();

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
    std::string case_insensitive_name;
    std::transform(module_info.name.begin() , module_info.name.end() , std::back_inserter(case_insensitive_name) , ::toupper);

    UUID id = FNV(case_insensitive_name);
    OE_DEBUG("Loading Lua script module {} [{}]" , module_info.name , id);

    if (loaded_modules.find(id) != loaded_modules.end()) {
      OE_ERROR("Attempting to load script module {} that is already loaded" , module_info.name);
      return nullptr;
    }

    for (const auto& path : module_info.paths) {
      if (!Filesystem::PathExists(path)) {
        OE_ERROR("Script module {} path {} does not exist" , module_info.name , path);
        return nullptr;
      }
    }

    loaded_modules[id] = new LuaScript(lua_state , module_info.paths);
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
