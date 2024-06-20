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
#include "scripting/cs/cs_module.hpp"
#include "scripting/lua/lua_module.hpp"

namespace other {
  
  struct LanguageModuleMetadata {
    UUID id;
    std::string name;
    Ref<LanguageModule> module = nullptr;
  };

  template <typename T> requires LangModule<T>
  LanguageModuleType ModuleTypeFromStaticType() {
    if constexpr (std::is_same_v<T , LuaModule>) {
      return LanguageModuleType::LUA_MODULE;
    } else if constexpr (std::is_same_v<T , CsModule>) {
      return LanguageModuleType::CS_MODULE;
    } else {
      return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }
  }

  class ScriptEngine {
    public:
      static void Initialize(Engine* engine , const ConfigTable& config);
      static void Shutdown();

      static void UpdateScripts();

      static std::string GetProjectAssemblyDir();

      static void ReloadAllScripts();

      static Ref<LanguageModule> GetModule(LanguageModuleType type);
      
      static ScriptModule* GetScriptModule(const std::string_view name);
      static ScriptModule* GetScriptModule(UUID id);

      static void SetAppContext(App* app);
      static App* GetAppContext();

      static std::map<UUID , LanguageModuleMetadata>& GetModules();

      /// because certain parts of the engine rely on specifically lua or c# scripts they need to be 
      ///   able to interface with those modules directly
      /// this is an unfortunate result of the fact that i am too lazy to write my own lua engine 
      ///   and so am using sol, hopefully go away one day
      template <typename T> requires LangModule<T>
      static Ref<T> GetModuleAs(LanguageModuleType type) {
        return Ref<T>(language_modules[type].module);
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
