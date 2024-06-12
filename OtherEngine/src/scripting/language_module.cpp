/**
 * \file scripting/language_module.cpp
 **/
#include "scripting/language_module.hpp"

#include <iterator>

namespace other {

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

  void LanguageModule::Update() {
    for (auto& [id , mod] : loaded_modules) {
      mod->Update();
    }
  }
      
} // namespace other
