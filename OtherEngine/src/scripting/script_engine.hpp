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
    LanguageModuleType type;
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

      /// because certain parts of the engine rely on specifically lua or c# scripts they need to be 
      ///   able to interface with those modules directly
      /// this is an unfortunate result of the fact that i am too lazy to write my own lua engine 
      ///   and so am using sol, hopefully go away one day
      template <typename T> requires LangModule<T>
      static Ref<T> GetModuleAs(LanguageModuleType type) {
        for (const auto& [id , mod] : language_modules) {
          if (type == mod.type) {
            return Ref<T>(mod.module);
          }
          
          return nullptr;
        }
      }

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
