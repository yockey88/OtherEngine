/**
 * \file scripting/script_defines.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_DEFINES_HPP
#define OTHER_ENGINE_SCRIPT_DEFINES_HPP

#include <array>
#include <string_view>

#include "core/defines.hpp"

namespace other {
  
  class Engine;
  class App;
  class LanguageModule;

  enum LanguageModuleType {
    CS_MODULE = 0 ,
    LUA_MODULE ,

    NUM_LANGUAGE_MODULES ,
    INVALID_LANGUAGE_MODULE = NUM_LANGUAGE_MODULES
  };

  constexpr static size_t kNumModules = NUM_LANGUAGE_MODULES;
  constexpr static std::array<std::string_view , kNumModules> kModuleNames = {
    /// to match the fact config parse use uppercase for case insensitivity
    "C#" , "LUA" , 
  };

  using FunctionModuleBuilder = Scope<LanguageModule>(*)();

  struct ModuleInfo {
    LanguageModuleType type;
    std::string_view name;
    uint64_t hash;

    constexpr ModuleInfo(LanguageModuleType t , std::string_view n) 
      : type(t) , name(n) , hash(FNV(n)) {}
  };

  constexpr static std::array<ModuleInfo , kNumModules> kModuleInfo = {
    ModuleInfo(CS_MODULE , kModuleNames[CS_MODULE]) ,
    ModuleInfo(LUA_MODULE , kModuleNames[LUA_MODULE]) ,
  };

} // namespace other

#endif // !OTHER_ENGINE_SCRIPT_DEFINES_HPP
