/**
 * \file project_settings.hpp
 **/
#ifndef OTHER_ENGINE_PROJECT_SETTINGS_HPP
#define OTHER_ENGINE_PROJECT_SETTINGS_HPP

#include <array>
#include <string_view>

namespace other {

  enum DefaultLanguageModules {
    CS ,
    LUA ,

    NUM_LANGUAGES ,
    INVALID_LANGUAGE = NUM_LANGUAGES
  };

  constexpr static size_t kNumLanguages = DefaultLanguageModules::NUM_LANGUAGES + 1;
  constexpr std::array<std::string_view, kNumLanguages> kLanguageModules = {
    "C#",
    "Lua",

    "Invalid"
  };

} // namespace other

#endif // !OTHER_ENGINE_PROJECT_SETTINGS_HPP
