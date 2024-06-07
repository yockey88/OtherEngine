/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

#include <string>
#include <map>

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "core/config.hpp"
#include "plugin/plugin_loader.hpp"
#include "scripting/language_module.hpp"

namespace other {

  class Engine;
  class App;
  class LanguageModule;
  
  enum LanguageModuleType {
    CS_MODULE = 0 ,
    LUA_MODULE ,

    NUM_LANGUAGE_MODULES ,
    INVALID_LANGUAGE_MODULE = NUM_LANGUAGE_MODULES
  };

  constexpr static size_t kNumDefaultModules = NUM_LANGUAGE_MODULES;
  constexpr static std::array<std::string_view , kNumDefaultModules> kDefaultModuleNames = {
    /// to match the fact config parse use uppercase for case insensitivity
    "C#" ,
    "LUA" , 
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

  struct LanguageModuleMetadata {
    UUID id;
    std::string path;
    std::string name;
    LanguageModule* module = nullptr;
    Scope<PluginLoader> loader = nullptr;
  };

  class ScriptEngine {
    public:
      static void Initialize(Engine* engine , const ConfigTable& config);
      static void Shutdown();
      
      static void LoadModule(const std::string_view name , const std::string_view path);
      static void UnloadModule(const std::string_view name);

      static bool ModuleLoaded(const std::string_view name);
      static bool ModuleLoaded(UUID id);

      static std::string GetProjectAssemblyDir();

      static LanguageModule* GetModule(const std::string_view name);
      static LanguageModule* GetModule(UUID id);

      static void SetAppContext(App* app);

      static const std::map<UUID , LanguageModuleMetadata>& GetModules() { return language_modules; }

    private:
      static Engine* engine_handle;
      static App* app_context;
      static ConfigTable config;

      static std::vector<std::string> module_paths;
      static std::map<UUID , LanguageModuleMetadata> language_modules;

      static void LoadDefaultModules();

      static void ReinitializeModule(const UUID& id);
      static void ReinitializeAllModules();
      
      static void UnloadModule(const UUID& id);
      static void UnloadAllModules();
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_ENGINE_HPP
