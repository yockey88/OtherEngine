/**
 * \file scripting/script_engine.hpp
*/
#include "scripting/script_engine.hpp"

#include <spdlog/cfg/helpers.h>

#include "core/logger.hpp"
#include "core/config_keys.hpp"

#include "scripting/language_module.hpp"
#include "scripting/script_defines.hpp"

#include "application/app_state.hpp"
#include "scripting/script_module.hpp"

namespace other {

  ConfigTable ScriptEngine::config;

  Ref<Scene> ScriptEngine::scene_context = nullptr;

  std::vector<ScriptObjectTag> ScriptEngine::object_tags;
  std::map<UUID , LanguageModuleMetadata> ScriptEngine::language_modules;
  std::map<UUID , Ref<ScriptModule>> ScriptEngine::loaded_modules;
  std::map<UUID , Ref<ScriptObject>> ScriptEngine::objects;

  constexpr static std::array<Ref<LanguageModule>(*)() , kNumModules> kModuleGetters = {
    []() -> Ref<LanguageModule> { return Ref<CsModule>::Create(); } ,
    []() -> Ref<LanguageModule> { return Ref<LuaModule>::Create(); } ,
    // []() -> Ref<LanguageModule> { return Ref<PythonModule>::Create(); } ,
  };

  void ScriptEngine::Initialize(const ConfigTable& cfg) {
    OE_DEBUG("ScriptEngine::Initialize: Initializing ScriptEngine");

    config = cfg;
    
    LoadModule(CS_MODULE);
    LoadModule(LUA_MODULE);
  }
  
  void ScriptEngine::LoadProjectModules() {
    auto project_metadata = AppState::ProjectContext();
    if (project_metadata == nullptr) {
      OE_ERROR("Failed to load project metadata");
      return;
    } else {
      OE_DEBUG("loading scripts modules from project metadata");
    }
    
    auto project_path = project_metadata->GetMetadata().file_path.parent_path();
    auto project_bin = project_metadata->GetMetadata().bin_dir;
    auto script_bin = project_metadata->GetMetadata().script_bin_dir;

    Path cs_prefix = project_path / project_bin;
    if (script_bin.has_value()) {
      cs_prefix /= script_bin.value();
    }
    //// FIXME: this feels really hacky???
    cs_prefix /= "net8.0";

    Path lua_prefix = project_metadata->GetMetadata().assets_dir / "scripts";

    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);

    OE_DEBUG("Loading C# script modules");
    LoadProjectModule(cs_language_module , kCsModuleSection , cs_prefix);

    OE_DEBUG("Loading Lua script modules");
    LoadProjectModule(lua_language_module , kLuaModuleSection , lua_prefix);
  }

  void ScriptEngine::Shutdown() {
    for (auto& [id , mod] : language_modules) {
      mod.module->Shutdown();
      mod.module = nullptr;
    }
  }
      
  void ScriptEngine::UnloadProjectModules() {
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);

    OE_DEBUG("Unloading Lua script modules");
    UnloadProjectModule(lua_language_module , kLuaModuleSection);
    
    OE_DEBUG("Unloading C# script modules");
    UnloadProjectModule(cs_language_module , kCsModuleSection);
  }
      
  std::string ScriptEngine::GetProjectAssemblyDir() {
    auto project_ctx = AppState::ProjectContext();
    OE_DEBUG("{}/bin/{}" , project_ctx->GetFilePath() , "Debug");

    return "";
  }
      
  void ScriptEngine::ReloadAllScripts() {
    std::vector<UUID> old_loaded_modules;
    
    for (auto& [id , mod] : loaded_modules) {
      old_loaded_modules.push_back(id);
    }

    objects.clear();
    loaded_modules.clear();
  
    for (auto& [id , mod] : language_modules) {
      mod.module->Reload();
    }

    object_tags.clear();
      
    /// @todo: clean this up
    for (auto& id : old_loaded_modules) {
      for (auto& [lid , lang] : language_modules) {
        if (lang.module->HasScript(id)) {
          auto& loaded_module = loaded_modules[id] = lang.module->GetScriptModule(id);
          auto objs = loaded_module->GetObjectTags();

          for (auto& o : objs) {
            object_tags.push_back(o);
          }
        }
      }
    }
  }
      
  Ref<LanguageModule> ScriptEngine::GetModule(LanguageModuleType type) {
    if (type >= LanguageModuleType::INVALID_LANGUAGE_MODULE) {
      return nullptr;
    }

    return Ref<LanguageModule>::Clone(language_modules[type].module);
  }
      
  const std::vector<ScriptObjectTag>& ScriptEngine::GetLoadedObjects() {
    return object_tags;
  }
  
  ScriptModule* ScriptEngine::GetScriptModule(const std::string_view name) {
    auto* smod = GetScriptModule(FNV(name));
    if (smod == nullptr) {
      OE_WARN("Failed to retrieve script module {}" , name); 
    }
    return smod;
  }

  ScriptModule* ScriptEngine::GetScriptModule(UUID id) {
    auto itr = std::find_if(loaded_modules.begin() , loaded_modules.end() , [&](const auto& module) -> bool {
      return module.first == id;
    });
    if (itr != loaded_modules.end()) {
      return itr->second.Raw();
    }

    for (auto& [lid , lang] : language_modules) {
      if (lang.module->HasScript(id)) {
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
      
  ScriptObject* ScriptEngine::GetScriptObject(UUID id) {
    auto itr = objects.find(id);
    if (itr != objects.end()) {
      return itr->second.Raw();
    }

    return nullptr;
  }
      
  ScriptObject* ScriptEngine::GetScriptObject(const std::string_view name) {
    std::string search_name{ name };
    std::string name_space = "";
    if (search_name.find("::") != std::string::npos) {
      auto colon = search_name.find_first_of(':');
      auto second_colon = search_name.find_last_of(':');

      name_space = search_name.substr(0 , colon);
      search_name = search_name.substr(second_colon + 1 , search_name.length() - name_space.length() + 2);
    }

    OE_DEBUG("Getting object {}::{}" , name_space , search_name);

    return GetScriptObject(search_name , name_space);
  }
      
  ScriptObject* ScriptEngine::GetScriptObject(const std::string_view name , const std::string_view nspace , const std::string_view mod_name) {
    if (mod_name.empty()) {
      return GetScriptObject(name , nspace , nullptr);
    }

    ScriptModule* mod = GetScriptModule(mod_name);
    if (mod == nullptr) {
      OE_ERROR("Failed to retrireve script module : {}" , mod_name);
      return nullptr;
    }

    return GetScriptObject(name , nspace , mod);
  }
      
  ScriptObject* ScriptEngine::GetScriptObject(const std::string_view name , const std::string_view nspace , ScriptModule* module) {
    UUID id = FNV(name);
    // ScriptObject* check_loaded = GetScriptObject(id);
    // if (check_loaded != nullptr) {
    //   check_loaded->InitializeScriptFields();
    //   check_loaded->InitializeScriptMethods();
    //   return check_loaded;
    // }

    // bool script_in_mod = module != nullptr;

    // /// if there is a module
    // if (script_in_mod) {
    //   /// check if the script is in the module
    //   script_in_mod = module->HasScript(name , nspace);
    // }

    // /// script_in_mod can only be true here if the module is not null and the script is in the module
    // if (script_in_mod) {
    //   auto* obj =  module->GetScriptObject(std::string{ name } , std::string{ nspace });
    //   if (obj == nullptr) {
    //     OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)" , nspace , name , module->ModuleName());
    //     return nullptr;
    //   }

    //   UUID id = FNV(name);
    //   objects[id] = obj;
    //   return obj;
    // }
    // /// if module is not null and its not in there bail early
    // else if (module != nullptr) {
    //   OE_ERROR("Failed to find script object {}::{} in module {}" , nspace , name , module->ModuleName());
    //   return nullptr;
    // }
    
    // /// else search the rest of the modules
    // ScriptObject* obj = nullptr;

    // /// first check modules already loaded/grabbed by client at some point
    // for (auto& [id , mod] : loaded_modules) {
    //   if (mod->HasScript(name , nspace)) {
    //     obj = mod->GetScriptObject(std::string{ name } , std::string{ nspace });
    //     if (obj == nullptr) {
    //       OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)" , nspace , name , module->ModuleName());
    //       return nullptr;
    //     } else {
    //       OE_DEBUG("Retrieved {}::{}" , nspace , name);
    //     }

    //     UUID id = FNV(name);
    //     objects[id] = obj;
    //   }
    // }

    // for (auto& [lid , lang] : language_modules) {
    //   for (auto& [sid , script] : lang.module->GetModules()) {
    //     if (script->HasScript(std::string{ name } , std::string{ nspace })) {
    //       loaded_modules[sid] = script;
    //       obj = script->GetScriptObject(std::string{ name } , std::string{ nspace });

    //       if (obj == nullptr) {
    //         OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)" , nspace , name , module->ModuleName());
    //         return nullptr;
    //       } else {
    //         OE_DEBUG("Retrieved {}::{}" , nspace , name);
    //       }

    //       UUID id = FNV(name);
    //       objects[id] = obj;
    //     }
    //   }
    // }

    // if (obj != nullptr) {
    //   obj->InitializeScriptFields();
    //   obj->InitializeScriptMethods();
    //   obj->OnBehaviorLoad();
    //   return obj;
    // }

    OE_ERROR("ScriptEngine::GetScriptObject({}.{} [{}])" , nspace , name, module->GetLanguage());
    return nullptr;
  }

  const std::map<UUID , Ref<ScriptObject>>& ScriptEngine::ReadLoadedObjects() {
    return objects;
  }

  void ScriptEngine::SetSceneContext(const Ref<Scene>& scene) {
    scene_context = scene;
  }

  Ref<Scene> ScriptEngine::GetSceneContext() {
    return scene_context;
  }
      
  std::map<UUID , LanguageModuleMetadata>& ScriptEngine::GetModules() {
    return language_modules;
  }

  Script ScriptEngine::LoadScriptsFromTable(const ConfigTable& table , const std::string_view section) {
    auto script_paths = table.Get(section , kScriptsValue);

    std::string script_key = std::string{ section } + "." + std::string{ kScriptValue };
    auto script_objs = table.GetKeys(script_key);

    auto project_path = AppState::ProjectContext()->GetMetadata().file_path.parent_path();
    auto assets_dir = AppState::ProjectContext()->GetMetadata().assets_dir;

    OE_DEBUG("Loading Editor Scripts");
    for (auto& mod : script_paths) {
      Path module_path = Path{ mod }; 
      LoadScriptModule(module_path);
    }

    return CollectObjects(table , script_objs , section);

  }
      
  LanguageModuleType ScriptEngine::StringToModuleType(const std::string_view name) {
    UUID id = FNV(name);
    return IdToModuleType(id);
  }
      
  LanguageModuleType ScriptEngine::IdToModuleType(UUID id) {
    switch (id.Get()) {
      case kModuleInfo[CS_MODULE].hash:
        return LanguageModuleType::CS_MODULE;
      case kModuleInfo[LUA_MODULE].hash:
        return LanguageModuleType::LUA_MODULE;
      return LanguageModuleType::INVALID_LANGUAGE_MODULE;
    }

    return LanguageModuleType::INVALID_LANGUAGE_MODULE;
  }
      
  void ScriptEngine::LoadModule(LanguageModuleType type) {
    language_modules[type].id = kModuleInfo[type].hash;
    language_modules[type].name = kModuleInfo[type].name;
    language_modules[type].module = kModuleGetters[type]();
    language_modules[type].module->Initialize();
  }

  void ScriptEngine::LoadProjectModule(Ref<LanguageModule>& module , const std::string_view config_tag , const Path& prefix_path) {
    OE_ASSERT(module != nullptr , "Failed to load project module, module was null!");

    auto project_metadata = AppState::ProjectContext();
    OE_ASSERT(project_metadata != nullptr , "Failed to load project metadata");

    std::string real_key = std::string{ kScriptEngineSection } + "." + std::string{ config_tag };
    auto modules = config.Get(real_key , kModulesValue);
  
    for (const auto& mod : modules) {
      if (mod == "") {
        continue;
      }

      Path path = mod;
      if (!prefix_path.empty()) {
        path = prefix_path / mod;
      }

      size_t ext = path.extension().string().find_last_of('/');
      if (ext == std::string::npos) {
        ext = 0;
      } else {
        ext += 1;
      }
      
      size_t length = path.string().length() - ext;
      std::string name = path.filename().string().substr(ext , length);
      name = name.substr(0 , name.find_last_of('.'));

      OE_DEBUG("Loading script module {} from [{}]" , name , module->GetLanguageType());
      module->LoadScriptModule({
        .name = name ,
        .paths = { path.string() } ,
      });
    }
  }
  
  void ScriptEngine::UnloadProjectModule(Ref<LanguageModule>& module , const std::string_view config_tag) {
    if (module != nullptr) {
      std::string real_key = std::string{ kScriptEngineSection } + "." + std::string{ config_tag };
      auto modules = config.Get(real_key , kModulesValue);
    
      for (const auto& mod : modules) {
        Path path = mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        OE_DEBUG(" > Unloading script module {}" , name);
        module->UnloadScript(name);
      }
    } else {
      OE_ERROR("Failed to unload script module, module was null!");
    }
  }

  LanguageModuleType ScriptEngine::ModuleTypeFromExtension(const std::string_view ext) {
    if (ext == ".dll") {
      return LanguageModuleType::CS_MODULE;
    } else if (ext == ".lua") {
      return LanguageModuleType::LUA_MODULE;
    }
    return LanguageModuleType::INVALID_LANGUAGE_MODULE;
  }

  void ScriptEngine::LoadScriptModule(Path& module_path) {
    const std::string fname = module_path.filename().string();
    const std::string mname = fname.substr(0 , fname.find_last_of('.'));
    const std::string ext = module_path.extension().string();
    
    OE_DEBUG("Loading Editor Script Module : {} ({})" , mname , module_path.string());

    language_modules[ModuleTypeFromExtension(ext)].module->LoadScriptModule({
      .name = mname ,
      .paths = { module_path.string() } ,
      .type = ScriptType::EDITOR_SCRIPT ,
    });
  }
  
  Script ScriptEngine::CollectObjects(const ConfigTable& table , const std::vector<std::string>& objects , const std::string_view section) {
    Script res {};
    for (const auto& obj : objects) {
      auto scripts = table.Get(section , obj);

      ScriptModule* mod = GetScriptModule(obj);
      OE_ASSERT(mod != nullptr , "Failed to get script module {}" , obj);

      for (auto& s : scripts) {
        OE_DEBUG("Attaching editor script");

        std::string nspace = "";
        std::string name = s;
        if (s.find("::") != std::string::npos) {
          nspace = s.substr(0 , s.find_first_of(":"));
          OE_DEBUG("Editor script from namespace {}" , nspace);

          name = s.substr(s.find_last_of(":") + 1 , s.length() - nspace.length() - 2);
          OE_DEBUG(" > with name {}" , name);
        }

        Ref<ScriptObject> inst = mod->GetScriptObject(name , nspace);
        OE_ASSERT(inst != nullptr , "Failed to get script {} from script module {}" , s , obj);
        std::string case_ins_name;
        std::transform(s.begin() , s.end() , std::back_inserter(case_ins_name) , ::toupper);

        UUID id = FNV(case_ins_name);
        res.data[id] = ScriptObjectData{
          .module = obj ,
          .obj_name = s ,
        };
        Ref<ScriptObject>& obj = res.scripts[id] = inst;
        obj->OnBehaviorLoad();
      }
    }

    return res;
  }

} // namespace other
