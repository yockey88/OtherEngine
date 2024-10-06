/**
 * \file scripting/script_object.cpp
 **/
#include "scripting/script_object.hpp"

#include "ecs/entity.hpp"

namespace  other {

  UUID ScriptObject::GetEntityId() const {
    return handles.entity_id;
  }

  const std::string_view ScriptObject::ScriptInstanceName() const {
    return script_instance_name;
  }

  const Opt<std::string> ScriptObject::NameSpace() const {
    return name_space; 
  }

  const std::string_view ScriptObject::Name() const {
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

} // namespace other
