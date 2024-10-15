/**
 * \file parsing\ini_parser.cpp
 */
#include "parsing/ini_parser.hpp"

#include <fstream>
#include <sstream>

#include "core/errors.hpp"
#include "core/logger.hpp"

namespace other {

  ConfigTable IniFileParser::Parse() {
    {
      std::ifstream file(file_path);
      if (!file.is_open()) {
        throw IniException("File not found");
      }

      std::stringstream ss;
      ss << file.rdbuf();
      if (ss.str().empty()) {
        return ConfigTable();
      }
      contents = ss.str();
    }

    do {
      switch (Peek()) {
        case '[':
          HandleSection();
          break;

        case '#':
          HandleComment();
          break;

        case '\n':
          break;

        case '*':
          ++index;
          if (AtEnd()) {
            throw IniException("Invalid key-value pair", IniError::FILE_PARSE_ERROR, line);
          }
          HandleKey(false);
          break;

        default:
          HandleKey(true);
          break;
      }

      ++index;
      line.clear();
    } while (index < contents.size());

    return table;
  }

  void IniFileParser::Trim(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char ch) {
                return !std::isspace(ch);
              }));
    str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch) {
                return !std::isspace(ch);
              }).base(),
              str.end());
  }

  void IniFileParser::TrimQuotes(std::string& str) {
    if (str[0] == '\"' && str[str.size() - 1] == '\"') {
      str = str.substr(1, str.size() - 2);
      in_string = true;
    }
  }

  void IniFileParser::ParseSection(const std::string& line) {
    if (line[0] != '[') {
      throw IniException("Invalid section", IniError::FILE_PARSE_ERROR);
    }

    if (line[line.size() - 1] != ']') {
      throw IniException("Invalid section", IniError::FILE_PARSE_ERROR);
    }

    std::string section = line.substr(1, line.size() - 2);
    Trim(section);

    if (section.empty()) {
      throw IniException("Empty section", IniError::FILE_PARSE_ERROR);
    }

    std::for_each(section.begin(), section.end(), ::toupper);

    table.Add(section);
    current_section = section;
  }

  void IniFileParser::ParseKeyValue(const std::string& line, bool allow_key_modifications) {
    if (!current_section.has_value()) {
      throw IniException("Key-value pair defined without section", IniError::FILE_PARSE_ERROR);
    }

    auto pos = line.find('=');
    if (pos == std::string::npos) {
      throw IniException("Invalid key-value pair", IniError::FILE_PARSE_ERROR);
    }

    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);

    Trim(key);
    TrimQuotes(value);

    Trim(value);
    TrimQuotes(value);

    /// remove trailing commas
    if (key.back() == ',') {
      key = key.substr(0, key.size() - 1);
    }

    if (key.empty()) {
      throw IniException("Empty key", IniError::FILE_PARSE_ERROR);
    }

    current_key = key;

    if (value.empty()) {
      throw IniException(fmtstr("Key {} has empty value", key), IniError::FILE_PARSE_ERROR);
    } else if (value[0] == '{') {
      ParseValueList(value, allow_key_modifications);
      return;
    }

    table.Add(current_section.value(), current_key.value(), value, in_string, allow_key_modifications);
    in_string = false;
  }

  void IniFileParser::ParseValueList(const std::string& line, bool allow_key_modifications) {
    if (!current_key.has_value()) {
      throw IniException("Value list defined without key", IniError::FILE_PARSE_ERROR);
    }

    if (line[line.size() - 1] != '}') {
      throw IniException("Unclosed value list", IniError::FILE_PARSE_ERROR);
    }

    std::string list = line.substr(1, line.size() - 2);
    std::stringstream ss(list);
    std::string value;
    while (std::getline(ss, value, ',')) {
      Trim(value);
      TrimQuotes(value);
      table.Add(current_section.value(), current_key.value(), value, in_string, allow_key_modifications);
      in_string = false;
    }
  }

  void IniFileParser::HandleComment() {
    ++index;
    if (!AtEnd()) {
      if (Peek() == '[') {
        ++index;
        while (!AtEnd()) {
          if (Peek() == '#') {
            ++index;
            if (!AtEnd() && Peek() == ']') {
              break;
            }
          }
          ++index;
        }
      } else {
        while (!AtEnd() && Peek() != '\n') {
          ++index;
        }
      }
    }
  }

  void IniFileParser::HandleSection() {
    while (!AtEnd() && Peek() != '\n') {
      line += Peek();
      ++index;
    }
    ParseSection(line);
  }

  void IniFileParser::HandleKey(bool allow_key_modifications) {
    while (!AtEnd() && Peek() != '=') {
      line += Peek();
      ++index;
    }

    /// save equals
    if (index >= contents.length()) {
      throw IniException("key value par without value", IniError::FILE_PARSE_ERROR);
    }

    line += Peek();
    ++index;

    /// save whitespace
    if (index >= contents.length()) {
      throw IniException("key value pair without value", IniError::FILE_PARSE_ERROR);
    }

    line += Peek();
    ++index;

    if (Peek() == '{') {
      while (!AtEnd() && Peek() != '}') {
        line += Peek();
        ++index;
      }

      /// save '}'
      if (index >= contents.length()) {
        throw IniException("key value pair without value", IniError::FILE_PARSE_ERROR);
      }

      line += Peek();
      ++index;
    } else {
      while (!AtEnd() && Peek() != '\n') {
        line += Peek();
        ++index;
      }
    }
    ParseKeyValue(line, allow_key_modifications);
  }

  bool IniFileParser::AtEnd() const {
    return index >= contents.size();
  }

  char IniFileParser::Peek() const {
    return contents[index];
  }

}  // namespace other
