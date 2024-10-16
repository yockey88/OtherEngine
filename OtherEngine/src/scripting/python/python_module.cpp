
/**
 * \file scripting/python/python_module.cpp
 **/
#include "scripting/python/python_module.hpp"

namespace other {

  bool PythonModule::Initialize() {
    return false;
  }
  
  void PythonModule::Shutdown() {}
  
  void PythonModule::Reload() {}
  
  Ref<ScriptModule> PythonModule::GetScriptModule(const std::string_view name) {
    return nullptr;
  }
  
  Ref<ScriptModule> PythonModule::GetScriptModule(const UUID& id) {
    return nullptr;
  }
  
  Ref<ScriptModule> PythonModule::LoadScriptModule(const ScriptMetadata& module_info) {
    return nullptr;
  }
  
  void PythonModule::UnloadScript(const std::string& name) {}
  
  std::string_view PythonModule::GetModuleName() const {
    return "Python"; 
  }

  std::string_view PythonModule::GetModuleVersion() const {
    return "0.0.1";
  }

} // namespace other
