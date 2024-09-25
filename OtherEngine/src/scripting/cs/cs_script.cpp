/**
 * \file scripting/cs/cs_script.cpp
 **/
#include "scripting/cs/cs_script.hpp"

#include "scripting/cs/cs_object.hpp"
#include "scripting/script_module.hpp"

namespace other {

  void CsScript::Initialize() {
    valid = false;
  }

  void CsScript::Shutdown() {
    if (!valid) {
      return;
    }

    valid = false;
  }

  void CsScript::Reload() {
  }

  bool CsScript::HasScript(UUID id) const {
    if (!valid) {
      return false;
    }

    return false;
  }
      
  bool CsScript::HasScript(const std::string_view name , const std::string_view nspace) const {
    if (!valid) {
      return false;
    }

    return false;
  }
  
  ScriptObject* CsScript::GetScript(const std::string& name , const std::string& nspace) {
    if (!valid) {
      return nullptr;
    }

    return new CsObject();
  }

  std::vector<ScriptObjectTag> CsScript::GetObjectTags() {
   return {};
  }

} // namespace other
