/**
 * \file lua_module.cpp
 **/
#include "lua_module.hpp"

#include "core/filesystem.hpp"

#include "lua_script.hpp"
#include <filesystem>

namespace other {

  LuaModule::LuaModule(Engine* engine) 
    : LanguageModule(engine) {
  }

  LuaModule::~LuaModule() {
  }

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

    load_success = true;
    return load_success;
  }

  bool LuaModule::Reinitialize() {
    OE_DEBUG("Reinitializing Lua Module");
    if (!load_success) {
      return Initialize();
    }

    for (auto& [id , module] : loaded_modules) {
      module->Shutdown();
    }

    for (auto& [id , module] : loaded_modules) {
      module->Initialize();
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

    std::vector<std::string> paths;
    paths.insert(paths.end() , core_files.begin() , core_files.end());
    paths.insert(paths.end() , module_info.paths.begin() , module_info.paths.end());

    loaded_modules[id] = new LuaScript(GetEngine() , paths);
    loaded_modules[id]->Initialize();
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

OE_API other::Plugin* CreatePlugin(other::Engine* engine) {
  other::Logger::SetLoggerInstance(engine->GetLog());
  return new other::LuaModule(engine);
}

OE_API void DestroyPlugin(other::Plugin* plugin) {
  delete plugin;
}
