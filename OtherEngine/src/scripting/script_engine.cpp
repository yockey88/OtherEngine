/**
 * \file scripting/script_engine.hpp
*/
#include "scripting/script_engine.hpp"

#include <spdlog/cfg/helpers.h>

#include "core/logger.hpp"
#include "core/engine.hpp"
#include "scripting/cs/cs_module.hpp"
#include "scripting/lua/lua_module.hpp"
#include "scripting/script_defines.hpp"

namespace other {

  Engine* ScriptEngine::engine_handle = nullptr;
  App* ScriptEngine::app_context = nullptr;

  ConfigTable ScriptEngine::config;

  std::map<UUID , LanguageModuleMetadata> ScriptEngine::language_modules;
  std::map<UUID , ScriptModule*> ScriptEngine::loaded_modules;
  std::map<UUID , ScriptObject*> ScriptEngine::objects;

  static Scope<LanguageModule> null_module = nullptr;

  constexpr static std::array<Ref<LanguageModule>(*)() , kNumModules> kModuleGetters = {
    []() -> Ref<LanguageModule> { return Ref<CsModule>::Create(); } ,
    []() -> Ref<LanguageModule> { return Ref<LuaModule>::Create(); } ,
  };

  void ScriptEngine::Initialize(Engine* engine , const ConfigTable& cfg) {
    OE_DEBUG("ScriptEngine::Initialize: Initializing ScriptEngine");

    config = cfg;
    engine_handle = engine;
    
    LoadModule(CS_MODULE);
    LoadModule(LUA_MODULE);
  }

  void ScriptEngine::Shutdown() {
    for (auto& [id , mod] : language_modules) {
      mod.module->Shutdown();
      mod.module = nullptr;
    }
  }

  void ScriptEngine::UpdateScripts() {
    for (auto& [id , mod] : language_modules) {
      mod.module->Update();
    }
  }
      
  std::string ScriptEngine::GetProjectAssemblyDir() {
    auto project_ctx = app_context->GetProjectContext();
    OE_DEBUG("{}/bin/{}" , project_ctx->GetFilePath() , "Debug");

    return "";
  }
      
  void ScriptEngine::ReloadAllScripts() {
    loaded_modules.clear();
    for (auto& [id , mod] : language_modules) {
      mod.module->Reload();
    }
  }

  Ref<LanguageModule> ScriptEngine::GetModule(const std::string_view name) {
    auto type = StringToModuleType(name);
    if (type == LanguageModuleType::INVALID_LANGUAGE_MODULE) {
      OE_DEBUG("Failed to retrieve language module {}" , name);
      return nullptr;
    }

    return Ref<LanguageModule>::Clone(language_modules[type].module);
  }

  Ref<LanguageModule> ScriptEngine::GetModule(UUID id) {
    auto type = IdToModuleType(id);
    if (type == LanguageModuleType::INVALID_LANGUAGE_MODULE) {
      OE_DEBUG("Failed to retrieve language module {}" , id);
      return nullptr;
    }

    return Ref<LanguageModule>::Clone(language_modules[type].module);
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

  void ScriptEngine::SetAppContext(App* app) {
    app_context = app;
  }

  App* ScriptEngine::GetAppContext() {
    return app_context;
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
    language_modules[type].type = type;
    language_modules[type].id = kModuleInfo[type].hash;
    language_modules[type].name = kModuleInfo[type].name;
    language_modules[type].module = kModuleGetters[type]();
    language_modules[type].module->Initialize();
  }

} // namespace other
