/**
 * \file scripting/language_module.cpp
 **/
#include "scripting/language_module.hpp"

#include <iterator>

namespace other {
      
  LanguageModuleType LanguageModule::GetLanguageType() const {
    return lang_type;
  }

  bool LanguageModule::HasScriptModule(const std::string_view name) const {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);
    UUID id = FNV(case_insensitive_name);
    return HasScriptModule(id);
  }

  bool LanguageModule::HasScriptModule(UUID id) const {
    return std::find_if(loaded_modules.begin() , loaded_modules.end() , [&id](const auto& modpair) -> bool {
      return modpair.first == id; 
    }) != loaded_modules.end();
  }

  const ScriptModuleInfo* LanguageModule::GetScriptModule(const std::string& name) const {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);
    UUID id = FNV(case_insensitive_name);
    return GetScriptModule(id);
  }

  const ScriptModuleInfo* LanguageModule::GetScriptModule(const UUID& id) const {
    auto itr = loaded_modules_data.find(id);
    if (itr != loaded_modules_data.end()) {
      return &loaded_modules_data.at(id);
    }

    return nullptr;
  }
      
  const std::map<UUID , ScriptModule*>& LanguageModule::GetModules() const {
    return loaded_modules;
  }
      
} // namespace other
