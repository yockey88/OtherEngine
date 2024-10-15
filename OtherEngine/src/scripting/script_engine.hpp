/**
 * \file scripting\script_engine.hpp
 */
#ifndef OTHER_ENGINE_SCRIPT_ENGINE_HPP
#define OTHER_ENGINE_SCRIPT_ENGINE_HPP

#include <map>
#include <string>
#include <string_view>

#include "core/config.hpp"
#include "core/rand.hpp"
#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "scene/scene.hpp"

#include "scripting/cs/cs_module.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/language_module.hpp"
#include "scripting/lua/lua_module.hpp"
#include "scripting/script_defines.hpp"
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
    if constexpr (std::is_same_v<T, LuaModule>) {
      return LanguageModuleType::LUA_MODULE;
    } else if constexpr (std::is_same_v<T, CsModule>) {
      return LanguageModuleType::CS_MODULE;
    } else {
      return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }
  }

  template <typename T>
    requires lang_module_t<T>
  LanguageModuleType ModuleTypeFromObjStaticType() {
    if constexpr (std::is_same_v<T, LuaObject>) {
      return LanguageModuleType::LUA_MODULE;
    } else if constexpr (std::is_same_v<T, CsObject>) {
      return LanguageModuleType::CS_MODULE;
    } else {
      return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }
  }

  // struct NativeObjectProxyRegistry {
  //   template <typename T>
  //   using ProxyMap = std::map<UUID , T>;

  //   template <typename T>
  //   static ProxyMap<T> proxy_map;
  // };

  // extern NativeObjectProxyRegistry native_object_registry;

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
    static Ref<ScriptObject> GetScriptObject(const std::string_view name);
    static Ref<ScriptObject> GetScriptObject(const std::string_view name, const std::string_view nspace);
    static Ref<ScriptObject> GetScriptObject(const std::string_view name, const std::string_view nspace, const std::string_view mod_name);

    // template <typename T>
    // static UUID RegisterNativeObject(T& obj) {
    //   UUID id = Random::GenerateUUID();
    //   native_object_registry.proxy_map<T>[id] = obj;
    // }

    template <typename SO>
      requires script_object_t<SO>
    static ScriptRef<SO> GetObjectRef(const std::string_view name, const std::string_view nspace) {
      // LanguageModuleType type = ModuleTypeFromObjStaticType<SO>();
      // auto& mod = language_modules[type];
      // if (mod.module == nullptr) {
      //   OE_ERROR("Failed to get module for object {}::{}", nspace, name);
      //   return nullptr;
      // }

      // Ref<ScriptObject> obj = nullptr;
      // for (auto& [id, script_mod] : mod.module->GetModules()) {
      //   if (script_mod->HasScript(name, nspace)) {
      //     obj = script_mod->GetScriptObject(name, nspace);
      //     if (obj == nullptr) {
      //       // OE_ERROR("Failed to get script object {}::{} from module {}" , nspace , name , script_mod->ModuleName());
      //       return nullptr;
      //     }

      //     return Ref<ScriptObject>::Cast<ScriptObjectHandle<SO>>(obj);
      //   }
      // }

      // OE_ERROR("Failed to find script object {}::{} in module {}", nspace, name, mod.module->ModuleName());
      return nullptr;
    }

    template <typename SO>
      requires script_object_t<SO>
    static ScriptRef<SO> GetObjectRef(const std::string_view name, const std::string_view nspace, const std::string_view mod_name) {
      Ref<ScriptObject> obj = GetScriptObject(name, nspace, mod_name);
      if (obj == nullptr) {
        OE_ERROR("Could not find script object {}::{} in module {}", nspace, name, mod_name);
        return nullptr;
      }

      LanguageModuleType type = ModuleTypeFromObjStaticType<SO>();
      if (type != obj->LanguageType()) {
        OE_ERROR("Script object {}::{} in module {} is not of type {} (it is of type [{}])", nspace, name, mod_name, type, obj->LanguageType());
        return nullptr;
      }

      return Ref<ScriptObject>::Cast<ScriptObjectHandle<SO>>(obj);
    }

    static const std::map<UUID, Ref<ScriptObject>>& ReadLoadedObjects();

    static void SetSceneContext(const Ref<Scene>& scene);
    static Ref<Scene> GetSceneContext();

    static std::map<UUID, LanguageModuleMetadata>& GetModules();
    static const std::vector<ScriptObjectTag>& GetLoadedObjects();
    static const std::vector<ScriptObjectTag>& GetLoadedEditorObjects();

    static Script LoadScriptsFromTable(const ConfigTable& table, const std::string_view section);

    /// because certain parts of the engine rely on specifically lua or c# scripts they need to be
    ///   able to interface with those modules directly
    /// this is an unfortunate result of the fact that i am lazy
    template <typename T>
      requires lang_module_t<T>
    static Ref<T> GetModuleAs(LanguageModuleType type) {
      return Ref<LanguageModule>::Cast<T>(language_modules[type].module);
    }

   private:
    static ConfigTable config;

    static Ref<Scene> scene_context;

    static std::vector<ScriptObjectTag> object_tags;
    static std::map<UUID, LanguageModuleMetadata> language_modules;
    static std::map<UUID, Ref<ScriptModule>> loaded_modules;
    static std::map<UUID, Ref<ScriptObject>> objects;

    static LanguageModuleType StringToModuleType(const std::string_view);
    static LanguageModuleType IdToModuleType(UUID id);

    static void LoadModule(LanguageModuleType type);

    static void LoadProjectModule(Ref<LanguageModule>& module, const std::string_view config_tag, const Path& prefix_path = "");
    static void UnloadProjectModule(Ref<LanguageModule>& module, const std::string_view config_tag);

    static LanguageModuleType ModuleTypeFromExtension(const std::string_view extension);
    static void LoadScriptModule(Path& module_path);

    static Script CollectObjects(const ConfigTable& table, const std::vector<std::string>& objects, const std::string_view section);

    static std::pair<std::string, std::string> ParseScriptName(const std::string_view name);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCRIPT_ENGINE_HPP
