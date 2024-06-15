/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

#include <string>
#include <map>

#include "core/uuid.hpp"
#include "core/ref.hpp"
#include "scripting/script_defines.hpp"
#include "scripting/language_module.hpp"

namespace other {
  
  struct LanguageModuleMetadata {
    UUID id;
    std::string name;
    Ref<LanguageModule> module = nullptr;
  };

  class ScriptEngine {
    public:
      static void Initialize(Engine* engine , const ConfigTable& config);
      static void Shutdown();

      static void UpdateScripts();

      static std::string GetProjectAssemblyDir();

      static void ReloadAllScripts();

      static Ref<LanguageModule> GetModule(const std::string_view name);
      static Ref<LanguageModule> GetModule(UUID id);
      
      static ScriptModule* GetScriptModule(const std::string_view name);
      static ScriptModule* GetScriptModule(UUID id);

      static void SetAppContext(App* app);
      static App* GetAppContext();

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
