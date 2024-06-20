/**
 * \file scripting/script_object.cpp
 **/
#include "scripting/script_object.hpp"

namespace  other {
      
  void ScriptObject::SetEntityId(UUID id) {
    entity_id = id;
    OnSetEntityId();
  }

  UUID ScriptObject::GetEntityId() const {
    return entity_id;
  }

  const std::string& ScriptObject::ScriptName() const {
    return script_module_name;
  }

  const Opt<std::string> ScriptObject::NameSpace() const {
    return name_space; 
  }

  const std::string& ScriptObject::Name() const {
    return script_name;
  }
      
  LanguageModuleType ScriptObject::LanguageType() const {
    return lang_type;
  }
      
  void ScriptObject::MarkCorrupt() {
    is_corrupt = true;
  }

  bool ScriptObject::IsCorrupt() const {
    return is_corrupt;
  }

  bool ScriptObject::IsInitialized() const {
    return is_initialized;
  }
  
  std::map<UUID , ScriptField>& ScriptObject::GetFields() {
    return fields; 
  }

  const std::map<UUID , ScriptField>& ScriptObject::GetFields() const {
    return fields; 
  }
      
  Opt<Value> ScriptObject::OnCallMethod(const std::string_view name , Parameter* args , uint32_t argc) {
    return std::nullopt; 
  }

} // namespace other
