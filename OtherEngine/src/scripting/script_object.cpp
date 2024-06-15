/**
 * \file scripting/script_object.cpp
 **/
#include "scripting/script_object.hpp"

namespace  other {

  const std::string& ScriptObject::Name() const {
    return script_name;
  }
      
  const std::string& ScriptObject::LanguageName() const {
    return language;
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
