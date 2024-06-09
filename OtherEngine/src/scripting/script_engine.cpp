/**
 * \file scripting/script_engine.hpp
*/
#include "scripting/script_engine.hpp"

#include "core/config_keys.hpp"
#include "core/logger.hpp"
#include "core/engine.hpp"
#include "core/filesystem.hpp"
#include <spdlog/cfg/helpers.h>

namespace other {

  Engine* ScriptEngine::engine_handle = nullptr;
  App* ScriptEngine::app_context = nullptr;

  ConfigTable ScriptEngine::config;

  std::vector<std::string> ScriptEngine::module_paths;
  std::map<UUID , LanguageModuleMetadata> ScriptEngine::language_modules;
  std::map<UUID , ScriptModule*> ScriptEngine::loaded_modules;
  std::map<UUID , ScriptObject*> ScriptEngine::objects;

  static Scope<LanguageModule> null_module = nullptr;

  void ScriptEngine::Initialize(Engine* engine , const ConfigTable& cfg) {
    OE_DEBUG("ScriptEngine::Initialize: Initializing ScriptEngine");

    config = cfg;
    engine_handle = engine;
    
    /// check if we should load the default modules, we always should unless specified otherwise
    auto default_modules = cfg.GetVal<bool>(kScriptingSection , kDefaultModulesValue);
    if (!default_modules.has_value() || true == default_modules.value()) {
      LoadDefaultModules();
    }

    auto modules = config.Get(kScriptingSection , kModulesValue);

    /// this loop is loading client loaded language modules so we skip any defaults that are already loaded
    ///  if this is a core module and its not loaded then we load it
    for (auto& module : modules) {
      // if (std::ranges::find(module_paths , module) != module_paths.end()) {
      //   continue;
      // }
    
      OE_WARN("MODULE LOADING NOT IMPLEMENTED YET : Should have loaded {}" , module);
    }
  }

  void ScriptEngine::Shutdown() {
    UnloadAllModules();
  }

  void ScriptEngine::LoadModule(const std::string_view name , const std::string_view path) {
    if (!Filesystem::PathExists(path)) {
      OE_WARN("ScriptEngine::LoadModule: Path {} for module {} does not exist" , path , name);
      return;
    }
    
    UUID id = FNV(name);
    if (language_modules.find(id) != language_modules.end()) {
      OE_WARN("ScriptEngine::LoadModule: Module {} already loaded" , name);
      return;
    }

    auto& metadata = language_modules[id] = {
      .id = id,
      .path = std::string{ path } ,
      .name = std::string{ name } ,
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

    metadata.module = reinterpret_cast<LanguageModule*>(plugin);
    if (!metadata.module->Initialize()) {
      OE_WARN("ScriptEngine::LoadModule: Failed to initialize module {}" , name);
      metadata.loader->Unload();
      metadata.loader = nullptr;
      language_modules.erase(id);
      return;
    } else {
      /// do other context loading here and save the module path
      module_paths.push_back(std::string{ path });
    }

    OE_DEBUG("ScriptEngine::LoadModule: Loaded module {}" , name);
  }

  void ScriptEngine::UnloadModule(const std::string_view name) {
    OE_DEBUG("ScriptEngine::UnloadModule: Unloading module {}" , name);
    auto hash = FNV(name);
    UnloadModule(hash);
  }

  bool ScriptEngine::ModuleLoaded(const std::string_view name) {
    auto hash = FNV(name);
    return ModuleLoaded(hash);
  }

  bool ScriptEngine::ModuleLoaded(UUID id) {
    return language_modules.find(id) != language_modules.end();
  }
      
  std::string ScriptEngine::GetProjectAssemblyDir() {
    auto project_ctx = app_context->GetProjectContext();
    OE_DEBUG("{}/bin/{}" , project_ctx->GetFilePath() , "Debug");

    return "";
  }
  
  LanguageModule* ScriptEngine::GetModule(const std::string_view name) {
    auto hash = FNV(name);
    return GetModule(hash);
  }

  LanguageModule* ScriptEngine::GetModule(UUID id) {
    if (language_modules.find(id) != language_modules.end()) {
      return language_modules[id].module;
    }

    return nullptr;
  }
      
  ScriptModule* ScriptEngine::GetScriptModule(const std::string_view name) {
    auto* smod = GetScriptModule(FNV(name));
    if (smod == nullptr) {
      OE_WARN("Failed to retrieve script module {}" , name); 
    }
    return smod;
  }

  ScriptModule* ScriptEngine::GetScriptModule(UUID id) {
    if (std::find_if(loaded_modules.begin() , loaded_modules.end() , [&](const auto& module) -> bool {
      return module.first == id;
    }) != loaded_modules.end()) {
      return loaded_modules[id];
    }

    for (const auto& [lid , lang] : language_modules) {
      if (lang.module->HasScriptModule(id)) {
        ScriptModule* mod = lang.module->GetScriptModule(id);
        OE_ASSERT(mod != nullptr , "Failed to retrieve script module from language module");

        if (!mod->IsValid()) {
          OE_WARN("Corrupt script module {}" , id);
          return nullptr;
        }

        loaded_modules[id] = mod;
        return mod;
      }
    }

    return nullptr;
  }

  void ScriptEngine::SetAppContext(App* app) {
    app_context = app;
  }

  void ScriptEngine::LoadDefaultModules() {
    OE_DEBUG("ScriptEngine::LoadDefaultModules: Loading core modules");

    for (size_t i = 0; i < kNumDefaultModules; ++i) {
      auto& info = kDefaultModules[i];
      Path engine_core = std::filesystem::absolute(kEngineCoreDir);

#ifdef OE_DEBUG_BUILD
        engine_core /= info.debug_path;
#else
        engine_core /= info.release_path;
#endif

      LoadModule(info.name.data() , engine_core.string());
    }

    return;
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
      OE_DEBUG("ScriptEngine::UnloadModule: Unloading module {}" , lang_module->GetModuleName());
      lang_module->Shutdown();
      language_modules[id].loader->Unload();
      language_modules.erase(id);
    }
  }

  void ScriptEngine::UnloadAllModules() {
    for (auto& [name , metadata] : language_modules) {
      OE_DEBUG("ScriptEngine::UnloadAllModules: Unloading module {}" , metadata.module->GetModuleName());
      metadata.module->Shutdown();
      metadata.loader->Unload();
    }
    language_modules.clear();
  }

} // namespace other
