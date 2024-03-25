/**
 * \file scripting/script_engine.hpp
*/
#include "scripting/script_engine.hpp"

#include "core/logger.hpp"
#include "core/engine.hpp"
#include "core/filesystem.hpp"

namespace other {

  Engine* ScriptEngine::engine_handle = nullptr;
  App* ScriptEngine::app_context = nullptr;

  ConfigTable ScriptEngine::config;

  std::map<UUID , LanguageModuleMetadata> ScriptEngine::language_modules;

  static Scope<LanguageModule> null_module = nullptr;

  void ScriptEngine::Initialize(Engine* engine , const ConfigTable& cfg) {
    config = cfg;
    engine_handle = engine;

    std::string script_engine_str = "SCRIPT-ENGINE";
    auto modules = config.Get(script_engine_str , "MODULES");
    for (auto& module : modules) {
      auto sec_name = script_engine_str + ":" + module;
      auto name = config.Get(sec_name , "NAME");
      auto path = config.Get(sec_name , "PATH");

      if (name.size() != 1) {
        OE_WARN("ScriptEngine::Initialize: No name provided for language-module: {}" , module);
        continue;
      }

      if (path.size() != 1) {
        OE_WARN("ScriptEngine::Initialize: No path provided for language-module: {}" , module);
        continue;
      }

      auto real_path = (std::filesystem::path(path[0]) / (name[0] + ".dll")).string();
      OE_DEBUG("ScriptEngine::Initialize: Loading module {} from path {}" , name[0] , real_path);
      LoadModule(name[0] , real_path);
    }
  }

  void ScriptEngine::Shutdown() {
    UnloadAllModules();

    language_modules.clear(); 
  }

  void ScriptEngine::LoadModule(const std::string& name , const std::string& path) {
    if (!Filesystem::FileExists(path)) {
      OE_WARN("ScriptEngine::LoadModule: Path {} for module {} does not exist" , path , name);
      return;
    }
    
    UUID id = FNV(name);
    if (language_modules.find(id) != language_modules.end()) {
      OE_WARN("ScriptEngine::LoadModule: Module {} already loaded" , name);
      return;
    }

    auto& metadata  = language_modules[id] = {
      .id = id,
      .path = path,
      .name = name ,
      .loader = GetPluginLoader(path)
    };

    if (metadata.loader == nullptr) {
      OE_WARN("ScriptEngine::LoadModule: Failed to load plugin loader for module {}" , name);
      language_modules.erase(id);
      return;
    }

    Plugin* plugin = metadata.loader->Load(engine_handle);
    if (plugin == nullptr) {
      OE_WARN("ScriptEngine::LoadModule: Failed to load plugin for module {}" , name);
      metadata.loader->Unload();
      metadata.loader = nullptr;
      language_modules.erase(id);
      return;
    }

    /// bad 
    metadata.module = reinterpret_cast<LanguageModule*>(plugin); // metadata.loader->Load<LanguageModule>();
    metadata.module->Initialize();

    OE_DEBUG("ScriptEngine::LoadModule: Loaded module {}" , name);
  }

  void ScriptEngine::UnloadModule(const std::string& name) {
    auto hash = FNV(name);
    UnloadModule(hash);
  }

  LanguageModule* ScriptEngine::GetModule(const std::string& name) {
    auto hash = FNV(name);
    return GetModule(hash);
  }

  LanguageModule* ScriptEngine::GetModule(const UUID& id) {
    if (language_modules.find(id) != language_modules.end()) {
      return language_modules[id].module;
    }

    return nullptr;
  }

  void ScriptEngine::SetAppContext(App* app) {
    app_context = app;
  }

  void ScriptEngine::ReinitializeModule(const UUID& id) {
    auto* lang_module = GetModule(id);
    if (lang_module != nullptr) {
      lang_module->Reinitialize();
    }
  }

  void ScriptEngine::ReinitializeAllModules() {
    for(auto& [name , metadata] : language_modules) {
      metadata.module->Reinitialize();
    }
  }

  void ScriptEngine::UnloadModule(const UUID& id) {
    auto* lang_module = GetModule(id);
    if (lang_module != nullptr) {
      lang_module->Shutdown();
      language_modules.erase(id);
    }
  }

  void ScriptEngine::UnloadAllModules() {
    auto itr = language_modules.begin();
    while (itr != language_modules.end()) {
      itr->second.module->Shutdown();
      itr = language_modules.erase(itr);
    }
    language_modules.clear();
  }

} // namespace other
