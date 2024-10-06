/**
 * \file ecs/components/script.cpp
 **/
#include "ecs/components/script.hpp"

#include <algorithm>
#include <iterator>

#include "core/config_keys.hpp"
#include "core/logger.hpp"
#include "scripting/cs/cs_object.hpp"
#include "scripting/script_engine.hpp"
#include "ecs/entity.hpp"

namespace other {

  UUID Script::AddScript(const std::string_view name , const std::string_view nspace , const std::string_view module) {
    std::string case_ins_name;
    std::ranges::transform(name , std::back_inserter(case_ins_name) , ::toupper);
    UUID id = FNV(case_ins_name);

    ScriptRef<CsObject> inst = ScriptEngine::GetObjectRef<CsObject>(name , nspace , module);
    if (inst == nullptr) {
      OE_ERROR("Failed to get script {} from script module {}" , name , module);
      return 0;
    } else {
      OE_DEBUG("Adding script {} [{} {}] to entity {}" , name , id , module , parent_handle->Name());
    }

    data[id] = ScriptObjectData{
      .module = std::string{ module } ,
      .obj_name = std::string{ name } ,
    };
    scripts[id] = inst;

    inst->SetHandles(parent_uuid , parent_id , parent_handle);
    if (!ValidateScripts()) {
      OE_ERROR("Script [{}::{} , {}] failed validation on entity {}" , name , nspace , module , parent_handle->Name());
    } else {
      OE_DEBUG(" > Script {}::{} [{}] added to entity {}" , nspace , name , id , parent_handle->Name());
    }

    return id;
  }    
  
  void Script::RemoveScript(UUID id) {
    auto it = scripts.find(id);
    if (it != scripts.end()) {
      scripts.erase(it);
      data.erase(id);
    }
  }

  void Script::RemoveScript(const std::string_view name) {
    std::string case_ins_name;
    std::ranges::transform(name , std::back_inserter(case_ins_name) , ::toupper);
    UUID id = FNV(case_ins_name);
    RemoveScript(id);
  }
  
  void Script::ApiCall(const std::string_view name) {
    for (auto& [id , obj] : scripts) {
      obj->CallMethod<void>(std::string{ name });
    }
  }
  
  void Script::ApiCall(const std::string_view name , float dt) {
    for (auto& [id , obj] : scripts) {
      obj->CallMethod<void , float>(std::string{ name } , std::forward<float>(dt));
    }
  }
  
  bool Script::ValidateScripts() {
    bool result = true;
    for (auto& [id , obj] : scripts) {
      bool handle_match = obj->GetProperty<void*>("NativeHandle") == (void*)parent_handle;
      bool eid_match = obj->GetProperty<uint32_t>("EntityID") == (uint32_t)parent_id;
      bool oid_match = obj->GetProperty<uint64_t>("ObjectID") == parent_uuid.Get();

      if (!handle_match) {
        OE_ERROR("NativeHandle mismatch for script {} [{:p} != {:p}]" , obj->Name() , obj->GetProperty<void*>("NativeHandle") , (void*)parent_handle);
      }

      if (!eid_match) {
        OE_ERROR("EntityID mismatch for script {} [{} != {}]" , obj->Name() , obj->GetProperty<uint32_t>("EntityID"), (uint32_t)parent_id);
      }

      if (!oid_match) {
        OE_ERROR("ObjectID mismatch for script {} [{} != {}]" , obj->Name() , obj->GetProperty<uint64_t>("ObjectID") , parent_uuid.Get());
      }
      
      result &= handle_match && eid_match && oid_match;
    }
    return result;
  }
  
  void Script::SetHandles() {
    for (auto& [id , obj] : scripts) {
      obj->SetHandles(parent_uuid, parent_id , (void*)parent_handle);
    }
  }

  void Script::Clear() {
    data.clear();
    scripts.clear();
  }

  void ScriptSerializer::Serialize(std::ostream& stream , Entity* entity , const Ref<Scene>& scene) const {
    const auto& script = entity->GetComponent<Script>();

    SerializeComponentSection(stream , entity , "script");
    for (const auto& [id , s] : script.GetScripts()) {
      stream << s->ScriptInstanceName() << " = { \"";
      if (s->NameSpace().has_value()) {
        stream << s->NameSpace().value() << "::";
      }
      stream << s->Name() << "\" }\n";
    }
  }

  void ScriptSerializer::Deserialize(Entity* entity , const ConfigTable& scene_table , Ref<Scene>& scene) const {
    OE_ASSERT(entity != nullptr && scene != nullptr , "Attempting to deserialize scripts into null entity or scene");
    auto& script = entity->AddComponent<Script>();
    std::string key_value = GetComponentSectionKey(entity->Name() , std::string{ kScriptValue });

    try {
      OE_DEBUG("Getting key value from section key : {}" , key_value);
      auto& mod_s = scene_table.GetKeys(key_value);
      OE_DEBUG("finding first script module {}" , key_value);

      for (auto& m : mod_s) {
        auto scripts = scene_table.Get(key_value , m);
        for (auto& s : scripts) {
          OE_DEBUG("Attaching {} to {}" , s , entity->Name());

          UUID sid = script.AddScript(s , "" , m);
          OE_DEBUG("Attached {} [{}]" , s , sid);

        }
      }

      OE_DEBUG("Attached {} scripts to entity {}" , script.GetScripts().size() , entity->Name());
    } catch (const std::exception& e) {
      OE_ERROR("Failed to attach component {} : {}" , key_value , e.what());
    } catch (...) {
      OE_ERROR("Unknown error while attaching component : {}" , key_value);
    }
  }

} // namespace other 
