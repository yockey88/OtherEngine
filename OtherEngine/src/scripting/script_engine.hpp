/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

#include <string>
#include <map>

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "core/ref.hpp"
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

  constexpr static size_t kNumModules = NUM_LANGUAGE_MODULES;
  constexpr static std::array<std::string_view , kNumModules> kModuleNames = {
    /// to match the fact config parse use uppercase for case insensitivity
    "C#" ,
    "LUA" , 
  };

  using FunctionModuleBuilder = Scope<LanguageModule>(*)();

  struct ModuleInfo {
    LanguageModuleType type;
    std::string_view name;
    uint64_t hash;

    constexpr ModuleInfo(LanguageModuleType t , std::string_view n) 
      : type(t) , name(n) , hash(FNV(n)) {}
  };

  constexpr static std::array<ModuleInfo , kNumModules> kModuleInfo = {
    ModuleInfo(CS_MODULE , kModuleNames[CS_MODULE]) ,
    ModuleInfo(LUA_MODULE , kModuleNames[LUA_MODULE]) ,
  };

  struct LanguageModuleMetadata {
    UUID id;
    std::string name;
    Ref<LanguageModule> module = nullptr;
  };

  class ScriptEngine {
    public:
      static void Initialize(Engine* engine , const ConfigTable& config);
      static void Shutdown();

      static void Reinitialize();

      static std::string GetProjectAssemblyDir();

      static void ReloadAllScripts();

      static Ref<LanguageModule> GetModule(const std::string_view name);
      static Ref<LanguageModule> GetModule(UUID id);
      
      static ScriptModule* GetScriptModule(const std::string_view name);
      static ScriptModule* GetScriptModule(UUID id);

      static void SetAppContext(App* app);

      static const std::map<UUID , LanguageModuleMetadata>& GetModules() { return language_modules; }

    private:
      static Engine* engine_handle;
      static App* app_context;
      static ConfigTable config;

      static std::map<UUID , LanguageModuleMetadata> language_modules;
      static std::map<UUID , ScriptModule*> loaded_modules;
      static std::map<UUID , ScriptObject*> objects;

      static LanguageModuleType StringToModuleType(const std::string_view);
      static LanguageModuleType IdToModuleType(UUID id);

      static void LoadModule(LanguageModuleType type);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_ENGINE_HPP
