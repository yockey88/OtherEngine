/**
 * \file testing_core/errors.hpp
 **/
#ifndef OTHER_TEST_ENGINE_ERRORS_HPP
#define OTHER_TEST_ENGINE_ERRORS_HPP

#include <source_location>
#include <sstream>
#include <string>
#include <string_view>

#include <nlohmann/detail/conversions/to_json.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>

#include "core/formatters.hpp"
#include "core/writer_reader.hpp"

namespace other {

  enum class ErrorLevel {
    WARNING = 0,
    NON_FATAL,
    FATAL,
    INTERNAL
  };

  struct ErrorMarker {
    size_t line;
    size_t column;
    std::string_view file;

    ErrorMarker() = default;
    ErrorMarker(size_t line, size_t column, std::string_view file)
        : line(line), column(column), file(file) {}
  };

  class ErrorDescription {
   public:
    ErrorDescription(ErrorLevel level, const std::string_view message, std::source_location loc)
        : marker(loc.line(), loc.column(), loc.file_name()), level(level), msg(message) {}

    ErrorMarker marker;
    ErrorLevel level;
    std::string msg;
  };

  struct OtherTestEngineError {
    OtherTestEngineError(ErrorLevel level, const std::string_view message, std::source_location loc = std::source_location::current())
        : error(level, message, loc) {}
    size_t num = 1;
    ErrorDescription error;
    bool process_must_halt = false;
  };

  template <>
  struct Writer<ErrorMarker> {
    std::ostream& operator()(std::ostream& os, const ErrorMarker& marker) const {
      os << marker.file << " @ [" << marker.line << " : " << marker.column << "]";
      return os;
    }

    nlohmann::json operator()(const ErrorMarker& marker) {
      return nlohmann::json::object({
        { "line", marker.line },
        { "column", marker.column },
        { "file", marker.file },
      });
    }
  };

  template <>
  struct Writer<ErrorDescription> {
    std::ostream& operator()(std::ostream& os, const ErrorDescription& desc) const {
      os << "----------------\n";
      os << "  - Level    | " << fmtstr("{}", desc.level) << "\n";
      os << "  - Message  | " << desc.msg << "\n";
      os << "  - Location | ";
      Writer<ErrorMarker>{}(os, desc.marker);
      os << "\n";
      os << "----------------\n";
      return os;
    }

    nlohmann::json operator()(const ErrorDescription& desc) {
      nlohmann::json marker_json = Writer<ErrorMarker>{}(desc.marker);
      return nlohmann::json::object({
        { "level", fmtstr("{}", desc.level) },
        { "msg", desc.msg },
        { "marker", marker_json },
      });
    }
  };

  template <>
  struct Writer<OtherTestEngineError> {
    std::ostream& operator()(std::ostream& os, const OtherTestEngineError& error) const {
      os << "Error (" << error.num << " : " << error.error.msg << ")\n";
      Writer<ErrorDescription>{}(os, error.error);
      os << "\n";
      return os;
    }

    nlohmann::json operator()(const OtherTestEngineError& error) {
      nlohmann::json marker_json = Writer<ErrorMarker>{}(error.error.marker);
      return nlohmann::json::object({
        { "num", error.num },
        { "error", error.error.msg },
        { "level", fmtstr("{}", error.error.level) },
        { "marker", marker_json },
      });
    }
  };

}  // namespace other

#endif  // !OTHER_TEST_ENGINE_ERRORS_HPP
