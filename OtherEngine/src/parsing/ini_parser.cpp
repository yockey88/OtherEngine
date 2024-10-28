/**
 * \file parsing\ini_parser.cpp
 */
#include "parsing/ini_parser.hpp"

#include <fstream>
#include <ranges>
#include <sstream>
#include <string>

#include "core/defines.hpp"
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
            throw IniException("Invalid key-value pair", IniError::FILE_PARSE_ERROR, current_line);
          }
          HandleKey(false);
          break;

        default:
          HandleKey(true);
          break;
      }

      ++index;
      current_line.clear();
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

  std::string IniFileParser::StripParens(const std::string& str) {
    return str |
      std::views::filter([](char c) { return c != '{' && c != '}'; }) |
      std::views::filter([](char c) { return c != '[' && c != ']'; }) |
      std::views::filter([](char c) { return c != '(' && c != ')'; }) |
      std::ranges::to<std::string>();
  }

  std::vector<std::string> IniFileParser::SplitOn(const std::string& str, char c) {
    return str |
      std::views::split(c) |
      std::ranges::to<std::vector<std::string>>();
  }

  std::pair<std::string, std::string> IniFileParser::SplitOnFirst(const std::string& str, char c) {
    auto first_half = str |
      std::views::take_while([c](char ch) { return ch != c; }) |
      std::ranges::to<std::string>();

    auto second_half = str |
      std::views::drop_while([c](char ch) { return ch != c; }) |
      std::views::drop(1) |
      std::views::all |
      std::ranges::to<std::string>();

    Trim(first_half);
    Trim(second_half);
    return { first_half, second_half };
  }

  void IniFileParser::PushKey(const std::string& key) {
    current_key.push(key);
    full_current_key = GetFullKey();
  }

  void IniFileParser::PopKey() {
    if (!current_key.empty()) {
      current_key.pop();
    }
    full_current_key = GetFullKey();
  }

  void IniFileParser::ParseSection(const std::string& line) {
    if (!current_key.empty()) {
      throw IniException(fmtstr("Invalid syntax, previous section invalid : {}", line), IniError::FILE_PARSE_ERROR);
    }

    if (line.size() <= 2 || !line.starts_with('[') || !line.ends_with(']')) {
      throw IniException(fmtstr("Invalid section : {}", line), IniError::FILE_PARSE_ERROR);
    }

    std::string section = StripParens(line);
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
      throw IniException(fmtstr("Key-value pair defined without section", line), IniError::FILE_PARSE_ERROR);
    }

    auto [k, v] = SplitOnFirst(line, '=');
    // std::cout << "k : " << k << " v : " << v << std::endl;
    if (v.empty()) {
      in_string = false;
      return;
    }
    if (v.starts_with('{') && !v.ends_with('}')) {
      v += "}";
    }

    ParseKey(k, allow_key_modifications);
    ParseValue(v, allow_key_modifications);
    current_key.pop();
    in_string = false;
  }

  void IniFileParser::ParseKey(const std::string& key, bool allow_key_modifications) {
    if (key.empty()) {
      throw IniException("Empty key", IniError::FILE_PARSE_ERROR);
    }

    std::string k = key;

    /// remove trailing commas
    if (k.back() == ',') {
      k = k.substr(0, k.size() - 1);
    }

    if (k.empty()) {
      throw IniException("Empty key", IniError::FILE_PARSE_ERROR);
    }

    PushKey(k);
  }

  void IniFileParser::ParseValue(const std::string& value, bool allow_key_modifications) {
    if (current_key.empty()) {
      throw IniException(fmtstr("Value defined without key : {}", value), IniError::FILE_PARSE_ERROR);
    }
    std::string v = value;

    TrimQuotes(v);
    if (v.empty()) {
      throw IniException(fmtstr("Key {} has empty value", full_current_key), IniError::FILE_PARSE_ERROR);
    }

    if (v[0] != '{') {
      table.Add(current_section.value(), full_current_key, v, in_string, allow_key_modifications);
    } else {
      /// resursively parses value list
      ParseValueList(v, allow_key_modifications);
    }
  }

  void IniFileParser::ParseValueList(const std::string& line, bool allow_key_modifications) {
    if (current_key.empty()) {
      throw IniException(fmtstr("Value list defined without key : {}", line), IniError::FILE_PARSE_ERROR);
    }

    if (!line.ends_with('}')) {
      throw IniException(fmtstr("Unclosed value list : {}", line), IniError::FILE_PARSE_ERROR);
    }

    std::string list = StripParens(line);
    std::stringstream ss(list);
    std::string value;

    /// if there are no '=' then this is a straight list { x_1 , x_2 , ... , x_n }
    if (!list.contains('=')) {
      while (std::getline(ss, value, ',')) {
        Trim(value);
        TrimQuotes(value);
        table.Add(current_section.value(), full_current_key, value, in_string, allow_key_modifications);
        in_string = false;
      }
      return;
    }
    /// if there are equals then this is a list of key-value pairs { x_1 = y_1 , x_2 = y_2 , ... , x_n = y_n }
    /// so we recursively call ParseValue to handle each key-value pair
    else {
      while (std::getline(ss, value, ',')) {
        Trim(value);
        ParseKeyValue(value, allow_key_modifications);
      }
    }
  }

  std::string IniFileParser::GetFullKey() const {
    if (current_key.empty()) {
      return "";
    }

    std::stack<std::string> temp = {};
    std::stack<std::string> temp2 = current_key;
    while (!temp2.empty()) {
      temp.push(temp2.top());
      temp2.pop();
    }

    std::string full_key = "";
    while (!temp.empty()) {
      full_key += temp.top();
      temp.pop();
      if (!temp.empty()) {
        full_key += ".";
      }
    }
    return full_key;
  }

  void IniFileParser::HandleComment() {
    Consume();
    if (!AtEnd()) {
      if (Match('[')) {
        while (!(Match('#') && Match(']'))) {
          Consume();
        }
        Consume();
      } else {
        while (!Match('\n')) {
          Consume();
        }
      }
    }
  }

  void IniFileParser::HandleSection() {
    ConsumeUntil('\n');
    ParseSection(current_line);
  }

  void IniFileParser::HandleKey(bool allow_key_modifications) {
    /// save equals
    if (!ConsumeUntil('=')) {
      throw IniException("key value pair without value", IniError::FILE_PARSE_ERROR);
    }

    current_line += Advance();

    /// save whitespace
    if (AtEnd()) {
      throw IniException("key value pair without value", IniError::FILE_PARSE_ERROR);
    }

    current_line += Advance();
    if (Peek() == '{') {
      std::stack<char> stack = {};
      stack.push('{');
      current_line += Advance();

      do {
        if (Peek() == '{') {
          stack.push('{');
        } else if (Peek() == '}') {
          if (stack.empty()) {
            break;
          }
          stack.pop();
        }
        current_line += Advance();
      } while (!stack.empty());
    } else {
      ConsumeUntil('\n');
    }
    ParseKeyValue(current_line, allow_key_modifications);
  }

  bool IniFileParser::AtEnd() const {
    return index >= contents.size();
  }

  char IniFileParser::Peek() const {
    if (AtEnd()) {
      return '\0';
    }
    return contents[index];
  }

  char IniFileParser::Previous() const {
    if (index == 0) {
      return '\0';
    }
    return contents[index - 1];
  }

  char IniFileParser::Advance() {
    char c = Peek();
    Consume();
    return c;
  }

  void IniFileParser::Consume() {
    if (!AtEnd()) {
      ++index;
    }
  }

  bool IniFileParser::ConsumeUntil(char c) {
    while (!AtEnd() && Peek() != c) {
      current_line += Advance();
    }
    return Check(c);
  }

  bool IniFileParser::Check(char c) {
    return Peek() == c;
  }

  bool IniFileParser::Match(char c) {
    char ch = Peek();
    if (ch == c) {
      Consume();
    }
    return ch == c;
  }

}  // namespace other
