/**
 * \file scripting/script_engine.hpp
*/
#include "scripting/script_engine.hpp"

#include "core/config_keys.hpp"
#include "core/logger.hpp"
#include "core/engine.hpp"
#include "core/filesystem.hpp"

namespace other {
namespace {

  enum LanguageModuleType {
    CS_MODULE = 0 ,
    LUA_MODULE ,

    NUM_LANGUAGE_MODULES ,
    INVALID_LANGUAGE_MODULE = NUM_LANGUAGE_MODULES
  };

  constexpr static size_t kNumDefaultModules = NUM_LANGUAGE_MODULES;
  constexpr static std::array<std::string_view , kNumDefaultModules> kDefaultModuleNames = {
    "C#" ,
    "LUA" , /// to match the fact config parse use uppercase for case insensitivity
  };

  struct ModuleInfo {
    LanguageModuleType type;
    std::string_view name;
    std::string_view debug_path;
    std::string_view release_path;

    constexpr ModuleInfo(LanguageModuleType t , std::string_view n , std::string_view dp , std::string_view rp) 
      : type(t) , name(n) , debug_path(dp) , release_path(rp) {}
  };

  /// these paths should always work because they are relative to the engine core directory and 
  /// should not be accessed outside of Filesystem::GetEngineCoreDir(...) calls and preferably should move to 
  /// somewhere that will work everywhere 
  constexpr static std::array<ModuleInfo , kNumDefaultModules> kDefaultModules = {
    ModuleInfo(CS_MODULE , kDefaultModuleNames[CS_MODULE] , "bin/Debug/CsModule/CsModule.dll" , "bin/Release/CsModule/CsModule.dll") ,
    ModuleInfo(LUA_MODULE , kDefaultModuleNames[LUA_MODULE] , "bin/Debug/LuaModule/LuaModule.dll" , "bin/Release/LuaModule/LuaModule.dll") ,
  };

} // namespace <anonymous>

  Engine* ScriptEngine::engine_handle = nullptr;
  App* ScriptEngine::app_context = nullptr;

  ConfigTable ScriptEngine::config;


  std::vector<std::string> ScriptEngine::module_paths;
  std::map<UUID , LanguageModuleMetadata> ScriptEngine::language_modules;

  static Scope<LanguageModule> null_module = nullptr;

  void ScriptEngine::Initialize(Engine* engine , const ConfigTable& cfg) {
    OE_DEBUG("ScriptEngine::Initialize: Initializing ScriptEngine");

    config = cfg;
    engine_handle = engine;
    
    LoadDefaultModules();

    std::string script_engine_str = kScriptingSection.data();
    auto modules = config.Get(script_engine_str , kModulesValue);

    /// this loop is loading client loaded language modules so we skip any defaults that are already loaded
    ///  if this is a core module and its not loaded then we load it
    for (auto& module : modules) {
      if (std::ranges::find(module_paths , module) != module_paths.end()) {
        continue;
      }
    }
  }

  void ScriptEngine::Shutdown() {
    UnloadAllModules();
  }

  void ScriptEngine::LoadModule(const std::string& name , const std::string& path) {
    if (!Filesystem::PathExists(path)) {
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

    metadata.module = reinterpret_cast<LanguageModule*>(plugin);
    if (!metadata.module->Initialize()) {
      OE_WARN("ScriptEngine::LoadModule: Failed to initialize module {}" , name);
      metadata.loader->Unload();
      metadata.loader = nullptr;
      language_modules.erase(id);
      return;
    } else {
      /// do other context loading here and save the module path
      module_paths.push_back(path);
    }

    OE_DEBUG("ScriptEngine::LoadModule: Loaded module {}" , name);
  }

  void ScriptEngine::UnloadModule(const std::string& name) {
    OE_DEBUG("ScriptEngine::UnloadModule: Unloading module {}" , name);
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

  void ScriptEngine::LoadDefaultModules() {
    OE_DEBUG("ScriptEngine::LoadDefaultModules: Loading default modules");

    if (auto val = config.GetVal<bool>(kScriptingSection , kLoadCoreModulesValue); val.has_value() && val.value()) {
      OE_DEBUG("ScriptEngine::LoadDefaultModules: Loading all core modules");
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

      /// we loaded all core modules so we can skip the loop below 
      return;
    }

    OE_DEBUG("ScriptEngine::LoadDefaultModules: Loading core modules from config");
    auto modules = config.Get(kScriptingSection , kLoadCoreModulesValue);

    /// here we load core modules, so if its a client module we skip it
    for (auto& module : modules) {
      OE_DEBUG("ScriptEngine::LoadDefaultModules: Attempting to load module {}" , module);
      if (std::ranges::find(kDefaultModuleNames , module) == kDefaultModuleNames.end()) {
        continue;
      }

      auto mod_info = std::ranges::find_if(kDefaultModules , [&module](const ModuleInfo& info) {
        return info.name == module;
      });
      if (mod_info != kDefaultModules.end()) {
        Path engine_core = std::filesystem::absolute(kEngineCoreDir);

#ifdef OE_DEBUG_BUILD
        engine_core /= mod_info->debug_path;
#else
        engine_core /= mod_info->release_path;
#endif

        println("Loading core module {} from path {}", mod_info->name , engine_core.string());

        if (!Filesystem::PathExists(engine_core.string())) {
          OE_WARN("ScriptEngine::LoadDefaultModules: Path {} for module {} does not exist" , engine_core.string() , mod_info->name);
          continue;
        } else {
          OE_DEBUG("ScriptEngine::LoadDefaultModules: Loading core module {}" , mod_info->name);
        }

        LoadModule(mod_info->name.data() , engine_core.string());
      }
    }
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
