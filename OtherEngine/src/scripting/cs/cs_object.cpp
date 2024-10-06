/**
 * \file scripting/cs/cs_object.cpp
*/
#include "scripting/cs/cs_object.hpp"

#include "core/logger.hpp"

using dotother::Type;

namespace other {
  
  CsObject::CsObject(ScriptModule* module , Type& type , UUID handle) 
      : ScriptObjectHandle(LanguageModuleType::CS_MODULE , module , handle , "[Empty Script Object]") ,
        type(type) {
    hosted_object = type.NewInstance();
  }

  void CsObject::InitializeScriptMethods() {
  }

  void CsObject::InitializeScriptFields() {
  } 

  void CsObject::UpdateNativeFields() {
  }

} // namespace other
