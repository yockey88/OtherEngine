/**
 * \file parsing\ini_parser.hpp
*/
#ifndef OTHER_ENGINE_INI_PARSER_HPP
#define OTHER_ENGINE_INI_PARSER_HPP

#include <string>
#include <optional>

#include "core/config.hpp"

namespace other {

  class IniFileParser {
    public:
      IniFileParser(const std::string& file_path) 
        : file_path(file_path) {};
      ~IniFileParser() = default;

      ConfigTable Parse();
  
    private:
      std::string file_path;

      std::optional<std::string> current_section = "";
      std::optional<std::string> current_key = "";

      size_t line = 1;
      size_t col = 1;

      ConfigTable table;

      void Trim(std::string& str);
      void TrimQuotes(std::string& str);

      void ParseSection(const std::string& line);
      void ParseKeyValue(const std::string& line);
      void ParseValueList(const std::string& list);
  };

} // namespace other

#endif // !OTHER_ENGINE_INI_PARSER_HPP
