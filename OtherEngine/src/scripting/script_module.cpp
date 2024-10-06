/**
 * \file scripting/script_module.cpp 
 **/
#include "scripting/script_module.hpp"

namespace other {
  
  LanguageModuleType ScriptModule::GetLanguage() const {
    return language;
  }
  
  const std::string& ScriptModule::ModuleName() const {
    return module_name;
  }

} // namespace other 