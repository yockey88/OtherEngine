/**
 * \file scripting/script_engine.hpp
*/
#include "scripting/script_engine.hpp"

#include <spdlog/cfg/helpers.h>

#include "core/logger.hpp"
#include "core/config_keys.hpp"

#include "scripting/script_defines.hpp"

#include "application/app_state.hpp"

namespace other {

  ConfigTable ScriptEngine::config;

  Ref<Scene> ScriptEngine::scene_context = nullptr;

  std::map<UUID , LanguageModuleMetadata> ScriptEngine::language_modules;
  std::map<UUID , ScriptModule*> ScriptEngine::loaded_modules;
  std::map<UUID , ScriptObject*> ScriptEngine::objects;

  constexpr static std::array<Ref<LanguageModule>(*)() , kNumModules> kModuleGetters = {
    []() -> Ref<LanguageModule> { return Ref<CsModule>::Create(); } ,
    []() -> Ref<LanguageModule> { return Ref<LuaModule>::Create(); } ,
  };

  void ScriptEngine::Initialize(const ConfigTable& cfg) {
    OE_DEBUG("ScriptEngine::Initialize: Initializing ScriptEngine");

    config = cfg;
    
    LoadModule(CS_MODULE);
    LoadModule(LUA_MODULE);

    auto project_metadata = AppState::ProjectContext();
    
    OE_DEBUG("Loading C# script modules");
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);
    if (cs_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kCsModuleSection };
      auto cs_modules = cfg.Get(real_key , kPathsValue);
    
      auto project_path = project_metadata->GetMetadata().file_path.parent_path();
      for (const auto& cs_mod : cs_modules) {
        Path path = project_path / "bin" / "Debug" / cs_mod; 
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        cs_language_module->LoadScriptModule({
          .name = name ,
          .paths = { path.string() } ,
        });    
      }
    }

    OE_DEBUG("Loading Lua script modules");
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);
    if (lua_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kLuaModuleSection };
      auto lua_modules = cfg.Get(real_key , kPathsValue);

      auto assets_path = project_metadata->GetMetadata().assets_dir;
      for (const auto& lua_mod : lua_modules) {
        Path path = assets_path / "scripts" / lua_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        lua_language_module->LoadScriptModule({
          .name = name ,
          .paths = { path.string() } ,
        });
      }
    }
  }

  void ScriptEngine::Shutdown() {
    OE_DEBUG("Unloading Lua script modules");
    Ref<LanguageModule> lua_language_module = ScriptEngine::GetModule(LUA_MODULE);
    if (lua_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kLuaModuleSection };
      auto lua_modules = config.Get(real_key , kPathsValue);

      for (const auto& lua_mod : lua_modules) {
        Path path = lua_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        lua_language_module->UnloadScriptModule(name);
      }
    } else {
      OE_ERROR("Failed to load lua script modules!");
    }
    
    OE_DEBUG("Unloading C# script modules");
    Ref<LanguageModule> cs_language_module = ScriptEngine::GetModule(CS_MODULE);
    if (cs_language_module != nullptr) {
      std::string real_key = std::string{ kScriptingSection } + "." + std::string{ kCsModuleSection };
      auto cs_modules = config.Get(real_key , kPathsValue);
    
      for (const auto& cs_mod : cs_modules) {
        Path path = cs_mod;
        std::string name = path.filename().string().substr(0 , path.filename().string().find_last_of('.'));

        OE_DEBUG(" > Unloading C# script module {}" , name);
        cs_language_module->UnloadScriptModule(name);
      }
    } else {
      OE_ERROR("Failed to load C# script modules!");
    }

    for (auto& [id , mod] : language_modules) {
      mod.module->Shutdown();
      mod.module = nullptr;
    }
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
      
    for (auto& id : old_loaded_modules) {
      for (auto& [lid , lang] : language_modules) {
        if (lang.module->HasScriptModule(id)) {
          loaded_modules[id] = lang.module->GetScriptModule(id);
          continue;
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
      
  std::vector<ScriptObjectTag> ScriptEngine::GetLoadedEditorObjects() {
    std::vector<ScriptObjectTag> loaded_objs;

    for (const auto& [id , mod] : language_modules) {
      auto& scripts = mod.module->GetModules();

      for (const auto& [sid , script] : scripts) {
        if (sid.Get() == FNV("C#-CORE")) {
          continue;
        }

        if (mod.module->GetScriptModule(sid)->type == ScriptModuleType::SCENE_SCRIPT) {
          continue;
        }

        auto objs = script->GetObjectTags();
        for (auto& o : objs) {
          loaded_objs.push_back(o);
        }
      }
    }

    return loaded_objs;
  }
      
  std::vector<ScriptObjectTag> ScriptEngine::GetLoadedObjects() {
    std::vector<ScriptObjectTag> loaded_objs;

    for (const auto& [id , mod] : language_modules) {
      auto& scripts = mod.module->GetModules();

      for (const auto& [sid , script] : scripts) {
        if (sid.Get() == FNV("C#-CORE")) {
          continue;
        }

        if (mod.module->GetScriptModule(sid)->type == ScriptModuleType::EDITOR_SCRIPT) {
          continue;
        }

        auto objs = script->GetObjectTags();
        for (auto& o : objs) {
          loaded_objs.push_back(o);
        }
      }
    }

    return loaded_objs;
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

    for (auto& [lid , lang] : language_modules) {
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
    auto itr = objects.find(id);
    if (itr != objects.end()) {
      return itr->second;
    }

    /// if given a module, find it in there
    if (module != nullptr && module->HasScript(name , nspace)) {
      auto* obj =  module->GetScript(std::string{ name } , std::string{ nspace });
      if (obj == nullptr) {
        OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)" , nspace , name , module->ModuleName());
        return nullptr;
      }

      UUID id = FNV(name);
      objects[id] = obj;
      return obj;

    /// else search the rest of the modules
    } else if (module == nullptr) {
      for (const auto& [id , mod] : loaded_modules) {
        if (mod->HasScript(name , nspace)) {
          auto* obj =  mod->GetScript(std::string{ name } , std::string{ nspace });
          if (obj == nullptr) {
            OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)" , nspace , name , module->ModuleName());
            return nullptr;
          } else {
            OE_DEBUG("Retrieved {}::{}" , nspace , name);
          }

          UUID id = FNV(name);
          objects[id] = obj;
          return obj;
        }
      }

      OE_DEBUG("Checking unloaded modules for {}::{}" , nspace , name);

      /// if we havent returned then it might be in an unloaded module
      for (const auto& [lid , lang] : language_modules) {
        for (const auto& [sid , script] : lang.module->GetModules()) {
          if (script->HasScript(std::string{ name } , std::string{ nspace })) {
            loaded_modules[sid] = script;
            auto obj = script->GetScript(std::string{ name } , std::string{ nspace });
            if (obj == nullptr) {
              OE_ERROR("Failed to retrieve script object {}::{} from module {} (module corrupted)" , nspace , name , module->ModuleName());
              return nullptr;
            } else {
              OE_DEBUG("Retrieved {}::{}" , nspace , name);
            }

            UUID id = FNV(name);
            objects[id] = obj;
            return obj;
          }
        }
      }

    }

    OE_ERROR("Failed to find script object {}::{}" , nspace , name);
    return nullptr;
  }

  const std::map<UUID , ScriptObject*>& ScriptEngine::ReadLoadedObjects() {
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

} // namespace other
