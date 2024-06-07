/**
 * \file ecs/components/script.cpp
 **/
#include "ecs/components/script.hpp"

#include "core/config_keys.hpp"
#include "core/logger.hpp"
#include "scripting/language_module.hpp"
#include "scripting/script_engine.hpp"
#include "ecs/entity.hpp"

namespace other {

  void ScriptSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {}

  void ScriptSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize scripts into null entity or scene");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kScriptingSection });

    // auto& script = entity->AddComponent<Script>();

    // auto cs_scripts = scene_table.Get(key_value , kDefaultModuleNames[CS_MODULE]);
    // auto lua_scripts = scene_table.Get(key_value , kDefaultModuleNames[LUA_MODULE]);
    // auto modules_to_load = scene_table.Get(key_value , kModulesValue);

    // if (cs_scripts.size() == 0 && lua_scripts.size() == 0 && modules_to_load.size() == 0) {
    //   return;
    // }

    // if (cs_scripts.size() > 0) {
    //   LoadScripts(entity , kDefaultModuleNames[CS_MODULE] , kDefaultModules[CS_MODULE].debug_path , cs_scripts);
    // }

    // if (lua_scripts.size() > 0) {
    //   LoadScripts(entity , kDefaultModuleNames[LUA_MODULE] , kDefaultModules[LUA_MODULE].debug_path , lua_scripts);
    // }

    // std::vector<std::string> loaded_mods{};
    // if (modules_to_load.size() > 0) {
    //   for (auto& mod : modules_to_load) {
    //     OE_WARN("CUSTOM LANGUAGE MODULE UNIMPLEMENTED : Should have loaded {}" , mod);
    //     // std::string module_key;
    //     // std::transform(mod.begin() , mod.end() , std::back_inserter(module_key) , ::toupper);
    //     //  > trim file path into just file name (path/module.dll > module)
    //     // loaded_mods.push_back(module_key);
    //     //
    //     // script_engine.load(mod);

    //     // load module into script engine
    //     // get 
    //     //  [<entity-name>.script] 
    //     //  <mod> = { .... }
    //     // LoadScripts(entity , module_key , mod , module_scripts); 
    //   }

    //   return;
    // }
  }
      
  void ScriptSerializer::LoadScripts(Entity* entity , const std::string_view mod_name , const std::string_view mod_path ,
                                     const std::vector<std::string>& scripts) const {
    if (!ScriptEngine::ModuleLoaded(mod_name)) {
      ScriptEngine::LoadModule(mod_name , mod_path);
    }
    
    LanguageModule* mod = ScriptEngine::GetModule(mod_name);
    if (mod == nullptr) {
      OE_ERROR("Failed to retrieve language module : {}" , mod_name);
      return;
    }

    std::string project_bin = ScriptEngine::GetProjectAssemblyDir();

    for (const auto& script : scripts) {
      mod->LoadScriptModule({
        .name = script ,
      }); 
    }
  }

} // namespace other 
