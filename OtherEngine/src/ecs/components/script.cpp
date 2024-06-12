/**
 * \file ecs/components/script.cpp
 **/
#include "ecs/components/script.hpp"

#include <algorithm>
#include <iterator>

#include "core/config_keys.hpp"
#include "core/logger.hpp"
#include "scripting/script_engine.hpp"
#include "ecs/entity.hpp"

namespace other {

  void ScriptSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {}

  void ScriptSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize scripts into null entity or scene");
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kScriptValue });

    /// for each script in entity
    ///   load script object from script module

    auto& script = entity->AddComponent<Script>();

    auto& mod_s = scene_table.GetKeys(key_value);
    for (auto& m : mod_s) {
      auto scripts = scene_table.Get(key_value , m);
      
      ScriptModule* mod = ScriptEngine::GetScriptModule(m);
      if (mod == nullptr) {
        OE_ERROR("Failed to find scripting module {} [{}] for entity {}" , m , FNV(m) , entity->Name());
        continue;
      }


      for (auto& s : scripts) {
        OE_DEBUG("attaching script {} to ent : {}" , s , entity->Name());

        ScriptObject* inst = mod->GetScript(s);
        if (inst == nullptr) {
          OE_ERROR("Failed to get script {} from script module {}" , s , m);
          continue;
        } else {
          OE_DEBUG("Retrieved {} from {}" , s , m);
        }

        std::string case_ins_name;
        std::transform(s.begin() , s.end() , std::back_inserter(case_ins_name) , ::toupper);

        UUID id = FNV(case_ins_name);
        script.data[id] = ScriptObjectData{
          .module = m ,
          .obj_name = s ,
        };
        script.scripts[id] = inst;
      }
    }

    OE_DEBUG("Attached {} scripts to entity {}" , script.scripts.size() , entity->Name());
  }

} // namespace other 
