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

} // namespace other