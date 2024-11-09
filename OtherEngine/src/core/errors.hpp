/**
 * \file core\errors.hpp
 */
#ifndef OTHER_ENGINE_ERRORS_HPP
#define OTHER_ENGINE_ERRORS_HPP

#include <array>
#include <source_location>
#include <stdexcept>
#include <string>
#include <string_view>

#include "core/defines.hpp"

namespace other {

  enum IniError {
    FILE_NOT_FOUND = 0,
    FILE_EMPTY,
    FILE_PARSE_ERROR,

    SECTION_NOT_FOUND,
    KEY_NOT_FOUND,

    NUM_INI_ERRORS
  };

  enum ShaderError {
    SHADER_NOT_FOUND = 0,
    SHADER_EMPTY,

    SHADER_COMPILATION,
    SHADER_LINKING,

    SYNTAX_ERROR,
    INVALID_VALUE,

    INVALID_SHADER_DIRECTIVE,
    INVALID_SHADER_TYPE,

    INVALID_SHADER_CTX,

    SHADER_TRANSPILATION,

    NUM_SHADER_ERRORS,
  };

  enum CoreErrors {
    INVALID_REF_CAST,

    NUM_CORE_ERRORS,
    INVALID_CORE_ERROR = NUM_CORE_ERRORS,
  };

  enum AssetPipelineCompilerErrorType {
    INVALID_COMMAND_KEYWORD,

    NUM_ASSET_PIPELINE_ERRORS,
    INVALID_ASSET_PIPELINE_ERROR = NUM_ASSET_PIPELINE_ERRORS,
  };

  class IniException : public std::runtime_error {
   public:
    IniException(const std::string_view message, std::source_location loc = std::source_location::current())
        : std::runtime_error(fmtstr("[ {} ] [{}:{} in {}]", message.data(), loc.line(), loc.column(), loc.file_name())) {}
    IniException(const std::string_view message, IniError error, std::source_location loc = std::source_location::current())
        : std::runtime_error(fmterr("[ {} ] : {} [{}:{} in {}]", message, error, loc.line(), loc.column(), loc.file_name()).data()) {}
    IniException(const std::string_view message, IniError error, const std::string_view curr_line, std::source_location loc = std::source_location::current())
        : std::runtime_error(fmterr("[ {} ] : {} [{}:{} in {}]\ncurr_line:\n{}", message, error, loc.line(), loc.column(), loc.file_name(), curr_line).data()) {}

    IniError error;

    const char* what() const noexcept override {
      return std::runtime_error::what();
    }
  };

  class ShaderException : public std::runtime_error {
   public:
    ShaderException(const std::string_view message)
        : std::runtime_error(message.data()) {}
    ShaderException(const std::string_view message, ShaderError error, uint32_t line, uint32_t col)
        : std::runtime_error(fmterr("[ {} ] : {} (at {}:{})", message, error, line, col)) {}
  };

  class InvalidRefCast : public std::runtime_error {
   public:
    InvalidRefCast(const std::type_info& t1, const std::type_info& t2)
        : std::runtime_error(fmterr("Invalid Ref Cast : {} -> {}", t1.name(), t2.name())) {}
  };

  class AssetPipelineCompilerError : public std::runtime_error {
   public:
    AssetPipelineCompilerError(const std::string_view message, AssetPipelineCompilerErrorType error, std::source_location loc = std::source_location::current())
        : std::runtime_error(fmterr("[ {} ] : {} [{}:{} in {}]", message, error, loc.line(), loc.column(), loc.file_name()).data()) {}
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ERRORS_HPP
