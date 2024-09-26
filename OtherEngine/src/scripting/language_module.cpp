/**
 * \file scripting/language_module.cpp
 **/
#include "scripting/language_module.hpp"

#include <iterator>

namespace other {
      
  LanguageModuleType LanguageModule::GetLanguageType() const {
    return lang_type;
  }

  bool LanguageModule::HasScript(const std::string_view name) const {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);
    UUID id = FNV(case_insensitive_name);
    return HasScript(id);
  }

  bool LanguageModule::HasScript(UUID id) const {
    return std::find_if(loaded_modules.begin() , loaded_modules.end() , [&id](const auto& modpair) -> bool {
      return modpair.first == id; 
    }) != loaded_modules.end();
  }

  const ScriptMetadata* LanguageModule::GetScriptMetadata(const std::string& name) const {
    std::string case_insensitive_name;
    std::transform(name.begin() , name.end() , std::back_inserter(case_insensitive_name) , ::toupper);
    UUID id = FNV(case_insensitive_name);
    return GetScriptMetadata(id);
  }

  const ScriptMetadata* LanguageModule::GetScriptMetadata(const UUID& id) const {
    auto itr = loaded_modules_data.find(id);
    if (itr != loaded_modules_data.end()) {
      return &loaded_modules_data.at(id);
    }

    return nullptr;
  }
      
  std::map<UUID , Ref<ScriptModule>>& LanguageModule::GetModules() {
    return loaded_modules;
  }
      
} // namespace other
