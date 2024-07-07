/**
 * \file core\errors.hpp
 */
#ifndef OTHER_ENGINE_ERRORS_HPP
#define OTHER_ENGINE_ERRORS_HPP

#include <array>
#include <string>
#include <string_view>
#include <stdexcept>

#include "core/defines.hpp"

namespace other {

  enum IniError {
    FILE_NOT_FOUND = 0,
    FILE_EMPTY ,
    FILE_PARSE_ERROR ,

    SECTION_NOT_FOUND ,
    KEY_NOT_FOUND ,

    NUM_INI_ERRORS
  };

  enum ShaderError {
    SHADER_NOT_FOUND = 0 ,
    SHADER_EMPTY ,

    SHADER_COMPILATION ,
    SHADER_LINKING ,

    SYNTAX_ERROR ,
    INVALID_VALUE ,

    INVALID_SHADER_DIRECTIVE ,

    NUM_SHADER_ERRORS ,
  };

  constexpr uint32_t kNumIniErrors = IniError::NUM_INI_ERRORS + 1;
  constexpr std::array<std::string_view , kNumIniErrors> kIniErrStrings = {
    "FILE NOT FOUND" ,
    "FILE EMPTY" ,
    "FILE PARSE ERROR" ,

    "SECTION NOT FOUND" ,
    "KEY NOT FOUND" ,

    "UNKNOWN ERROR"
  };
  
  constexpr uint32_t kNumShaderErrors = ShaderError::NUM_SHADER_ERRORS + 1;
  constexpr std::array<std::string_view , kNumShaderErrors> kShaderErrStrings = {
    "SHADER_NOT_FOUND" ,
    "SHADER_EMPTY" ,

    "SHADER_COMPILATION" ,
    "SHADER_LINKING" ,

    "SYNTAX ERROR" ,
    "INVALID_VALUE" ,

    "UNKNOWN ERROR"
  };

  class IniException : public std::runtime_error {
    public:
      IniException(const std::string_view message) 
        : std::runtime_error(message.data()) {};
      IniException(const std::string_view message , IniError error) 
        : std::runtime_error(fmterr("[ {} ] : {})" , message , kIniErrStrings[error]).data()) {};

      IniError error;

      const char* what() const noexcept override {
        return std::runtime_error::what();
      }
  };

  class ShaderException : public std::runtime_error {
    public:
      ShaderException(const std::string_view message) 
        : std::runtime_error(message.data()) {}
      ShaderException(const std::string_view message , ShaderError error) 
        : std::runtime_error(fmterr("[ {} ] : {}" , message , kShaderErrStrings[error].data())){}
  };

} // namespace other

#endif // !OTHER_ENGINE_ERRORS_HPP
