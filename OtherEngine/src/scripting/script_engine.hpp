/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

#include <string>
#include <map>
#include <string_view>

#include "core/config.hpp"
#include "core/uuid.hpp"
#include "core/ref.hpp"

#include "scene/scene.hpp"

#include "scripting/script_defines.hpp"
#include "scripting/language_module.hpp"
#include "scripting/cs/cs_module.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/lua/lua_module.hpp"
#include "scripting/script_module.hpp"
#include "scripting/script_object.hpp"

namespace other {
  
  struct LanguageModuleMetadata {
    UUID id;
    std::string name;
    Ref<LanguageModule> module = nullptr;
  };

  template <typename T> 
    requires lang_module_t<T>
  LanguageModuleType ModuleTypeFromModStaticType() {
    if constexpr (std::is_same_v<T , LuaModule>) {
      return LanguageModuleType::LUA_MODULE;
    } else if constexpr (std::is_same_v<T , CsModule>) {
      return LanguageModuleType::CS_MODULE;
    } else {
      return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }
  }

  template <typename T> 
    requires lang_module_t<T>
  LanguageModuleType ModuleTypeFromObjStaticType() {
    if constexpr (std::is_same_v<T , LuaObject>) {
      return LanguageModuleType::LUA_MODULE;
    } else if constexpr (std::is_same_v<T , CsObject>) {
      return LanguageModuleType::CS_MODULE;
    } else {
      return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }
  }

  class ScriptEngine {
    public:
      static void Initialize(const ConfigTable& config);
      static void LoadProjectModules();

      static void Shutdown();
      static void UnloadProjectModules();

      static std::string GetProjectAssemblyDir();

      static void ReloadAllScripts();

      static Ref<LanguageModule> GetModule(LanguageModuleType type);
      
      static Ref<ScriptModule> GetScriptModule(const std::string_view name);
      static Ref<ScriptModule> GetScriptModule(UUID id);

      static Ref<ScriptObject> GetScriptObject(UUID id);
      static Ref<ScriptObject> GetScriptObject(const std::string_view name , const std::string_view nspace);
      static Ref<ScriptObject> GetScriptObject(const std::string_view name , const std::string_view nspace , const std::string_view mod_name);

      template <script_object_t SO>
      static ScriptRef<SO> GetScriptObject(const std::string_view name) {
        const auto [search_name , name_space] = ParseScriptName(name);
        LanguageModuleType type = ModuleTypeFromObjStaticType<SO>();

        OE_DEBUG("Getting object {}::{} [{}]" , name_space , search_name , type);
        if (type == LanguageModuleType::INVALID_LANGUAGE_MODULE) {
          OE_ASSERT(false , "Failed to get module type for script object {}::{}" , name_space , name);
          return nullptr;
        }

        for (auto& [id , mod] : language_modules[type].module->GetModules()) {
          if (!mod->HasScript(search_name , name_space)) {
            continue;
          }

          loaded_modules[FNV(mod->ModuleName())] = mod;
          Ref<ScriptObject> obj = mod->GetScriptObject(search_name , name_space);
          if (obj != nullptr) {
            return Ref<ScriptObject>::Cast<SO>(obj);
          }
        }
        
        OE_ERROR("ScriptEngine::GetScriptObject({}.{}) -> failed to find script" , name_space , search_name);
        return nullptr;
      }
      
      static const std::map<UUID , Ref<ScriptObject>>& ReadLoadedObjects();

      static void SetSceneContext(const Ref<Scene>& scene);
      static Ref<Scene> GetSceneContext();

      static std::map<UUID , LanguageModuleMetadata>& GetModules();
      static const std::vector<ScriptObjectTag>& GetLoadedObjects();
      static const std::vector<ScriptObjectTag>& GetLoadedEditorObjects();

      static Script LoadScriptsFromTable(const ConfigTable& table , const std::string_view section);

      /// because certain parts of the engine rely on specifically lua or c# scripts they need to be 
      ///   able to interface with those modules directly
      /// this is an unfortunate result of the fact that i am too lazy to write my own lua engine 
      ///   and so am using sol
      template <typename T> requires lang_module_t<T>
      static Ref<T> GetModuleAs(LanguageModuleType type) {
        return Ref<T>(language_modules[type].module);
      }

    private:
      static ConfigTable config;

      static Ref<Scene> scene_context;

      static std::vector<ScriptObjectTag> object_tags;
      static std::map<UUID , LanguageModuleMetadata> language_modules;
      static std::map<UUID , Ref<ScriptModule>> loaded_modules;
      static std::map<UUID , Ref<ScriptObject>> objects;

      static LanguageModuleType StringToModuleType(const std::string_view);
      static LanguageModuleType IdToModuleType(UUID id);

      static void LoadModule(LanguageModuleType type);

      static void LoadProjectModule(Ref<LanguageModule>& module , const std::string_view config_tag , const Path& prefix_path = "");
      static void UnloadProjectModule(Ref<LanguageModule>& module , const std::string_view config_tag);

      static LanguageModuleType ModuleTypeFromExtension(const std::string_view extension);
      static void LoadScriptModule(Path& module_path);

      static Script CollectObjects(const ConfigTable& table , const std::vector<std::string>& objects , const std::string_view section);

      static std::pair<std::string , std::string> ParseScriptName(const std::string_view name);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_ENGINE_HPP
