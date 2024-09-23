/**
 * \file scripting/lua/lua_module.cpp
 **/
#include "scripting/lua/lua_module.hpp"

#include "core/filesystem.hpp"
#include "application/app_state.hpp"

#include "scripting/lua/lua_bindings.hpp"
#include "scripting/lua/lua_script.hpp"
#include <cctype>
#include <iterator>

namespace other {

  LuaScript* LuaModule::GetRawScriptHandle(const std::string_view name) {
    UUID id = FNV(name);
    auto* module = GetScriptModule(id);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
    }

    return static_cast<LuaScript*>(module);
  }

  bool LuaModule::Initialize() {
    OE_DEBUG("Initializing Lua Module");
    OE_DEBUG("Lua Module Initialized");

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
    Shutdown();
    Initialize();

    for (const auto& [id , data] : loaded_modules_data) {
      LoadScriptModule(data);
    }

    load_success = true;
  }

  ScriptModule* LuaModule::GetScriptModule(const std::string& name) {
    std::string case_ins_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_ins_name) , ::toupper);

    UUID id = FNV(case_ins_name);
    auto* module = GetScriptModule(id);
    if (module == nullptr) {
      OE_ERROR("Script module {} not found" , name);
    }

    return module;
  }

  ScriptModule* LuaModule::GetScriptModule(const UUID& id) {
    auto itr = loaded_modules.find(id);
    if (itr != loaded_modules.end()) {
      return itr->second;
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

    
    auto project = AppState::ProjectContext();
    Path real_path = project->GetMetadata().assets_dir / module_info.paths[0];
    if (!Filesystem::PathExists(real_path)) {
      OE_ERROR("Script module {} ({}) does not exist" , module_info.name , real_path.string());
      return nullptr;
    }

    OE_DEBUG("Script module {} ({}) loaded" , module_info.name , real_path.string());

    Path mod_path = module_info.paths[0];
    std::string mod_path_str = mod_path.filename().string();
    std::string mod_name = mod_path_str.substr(0 , mod_path_str.find_last_of('.'));
    loaded_modules[id] = new LuaScript(real_path.string() , mod_name);
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
