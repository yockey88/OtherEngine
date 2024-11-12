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

#include "parsing/asset_pipeline_compiler.hpp"
#include "parsing/parser.hpp"

namespace other {

  ConfigTable IniFileParser::Parse() {
    if (!file_path.has_value()) {
      throw IniException("No file path set for IniFileParser", IniError::NO_SRC_PROVIDED);
    }

    std::ifstream file(*file_path);
    if (!file.is_open()) {
      throw IniException("File not found", IniError::FILE_NOT_FOUND);
    }

    std::stringstream ss;
    ss << file.rdbuf();
    if (ss.str().empty()) {
      return ConfigTable();
    }

    /// Call to parse will force parser reset once table is constructed and return,
    ///   save anything needed here before parsing
    Path p = *file_path;
    ConfigTable res = Parse(ss.str());
    res.SetPath(p);
    return res;
  }

  ConfigTable IniFileParser::Parse(const std::string_view src) {
    contents = src;

    do {
      switch (Peek()) {
        case '[':
          HandleSection();
          break;

        case '#':
          HandleComment();
          break;

        case '*':
          ++index;
          if (AtEnd()) {
            throw IniException("'*' found at <eof>", IniError::INVALID_KEY, current_line);
          }
          HandleKey(false);
          break;

        /// nothing on whitespace
        case '\n':
        case ' ':
          break;

        default:
          HandleKey(true);
          break;
      }

      ++index;
      current_line.clear();
    } while (index < contents.size());

    ConfigTable res = table;
    Reset();
    return res;
  }

  void IniFileParser::Reset() {
    file_path = std::nullopt;
    contents = "";
    current_line = "";
    current_section = "";
    current_key = {};
    full_current_key = "";
    index = 0;
    table = ConfigTable();
    in_string = false;
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
      throw IniException(fmtstr("Key unclosed, previous section invalid, current key : {}", current_key.top()), IniError::UNCLOSED_KEY_VALUE);
    }

    if (line.size() <= 2) {
      throw IniException(fmtstr("Invalid section header : {}", line), IniError::INVALID_SECTION);
    }

    std::string section = StripParens(line);
    Trim(section);

    std::for_each(section.begin(), section.end(), ::toupper);

    table.Add(section);
    current_section = section;
  }

  void IniFileParser::ParseScriptSection(const std::string& line) {
    if (!current_section.has_value()) {
      throw IniException(fmtstr("Unexpected scriptable section header, previous section unclosed : {}", *current_section), IniError::UNCLOSED_SECTION);
    }

    std::string l = line;
    Trim(l);
    if (l.size() <= 2 || !l.starts_with('[') || !l.ends_with("]")) {
      throw IniException(fmtstr("Invalid scriptable section header : {}", l), IniError::INVALID_SECTION);
    }

    std::string section = StripParens(l);
    Trim(section);

    std::for_each(section.begin(), section.end(), ::toupper);
    /// save scriptable section to table in a way it can be easily invoked later

    // table.Add(section);
    current_section = section;

    ConsumeUntil('{');
    if (!Check('{')) {
      throw IniException(fmtstr("Invalid section header : {}", *current_section), IniError::INVALID_SECTION);
    }
    Consume();

    ConsumeUntil('\n');
    if (!Check('\n')) {
      throw IniException(fmtstr("Invalid section header : {}", *current_section), IniError::INVALID_SECTION);
    }
    Consume();

    current_line.clear();
    /// advance until here cause we want to capture the entire body of the section
    AdvanceUntil('}');
    if (!Check('}')) {
      throw IniException(fmtstr("Failed to match closing brace : {}", line), IniError::UNCLOSED_SCRIPTABLE_SECTION);
    }
    Consume();

    /// parse dsl script and store executable structure in table

    AssetPipelineCompiler compiler;
    auto pipeline = compiler.CompileSource(current_line);
    /// table.AddScriptableSection(section, pipeline);

    /// clear current line
    current_line.clear();

    /// we can clear this here because we've compiled the body of the section
    current_section = std::nullopt;
  }

  void IniFileParser::ParseKeyValue(const std::string& line, bool allow_key_modifications) {
    if (!current_section.has_value()) {
      throw IniException(fmtstr("Key-value pair defined without section : {}", line), IniError::KEY_VALUE_WITHOUT_SECTION);
    }

    auto [k, v] = SplitOnFirst(line, '=');
    if (k.starts_with('*')) {
      k = k.substr(1);
      allow_key_modifications = false;
    }

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
      throw IniException("Empty key", IniError::INVALID_KEY);
    }

    std::string k = key;

    /// remove trailing commas
    if (k.back() == ',') {
      k = k.substr(0, k.size() - 1);
    }

    if (k.empty()) {
      throw IniException("Empty key", IniError::INVALID_KEY);
    }

    PushKey(k);
  }

  void IniFileParser::ParseValue(const std::string& value, bool allow_key_modifications) {
    if (current_key.empty()) {
      throw IniException(fmtstr("Value defined without key : {}", value), IniError::VALUE_WITHOUT_KEY);
    }
    std::string v = value;

    TrimQuotes(v);
    if (v.empty()) {
      throw IniException(fmtstr("Key {} has empty value", full_current_key), IniError::VALUE_WITHOUT_KEY);
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
      throw IniException(fmtstr("Value list defined without key : {}", line), IniError::VALUE_WITHOUT_KEY);
    }

    if (!line.ends_with('}')) {
      throw IniException(fmtstr("Unclosed value list : {}", line), IniError::UNCLOSED_VALUE_LIST);
    }

    std::string list = StripParens(line);
    Trim(list);
    /// strip trailing commas
    if (list.ends_with(',')) {
      list = list.substr(0, list.size() - 1);
    }

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
    current_line.clear();

    Ref<Parser<void>> skip_comment = SkipUntil('\n');
    Ref<Parser<void>> skip_newline = Skip('\n');
    Ref<Parser<void>> skip_any = SkipAny();

    /// consumes '#'
    Consume();
    if (!AtEnd()) {
      /// parse block comment
      if (Match('[')) {
        while (!(Match('#') && Match(']'))) {
          Consume();
        }
      }
      /// parse line comment
      else {
        while (!Check('\n') && !AtEnd()) {
          Consume();
        }
      }
    }
    current_line.clear();
  }

  void IniFileParser::HandleSection() {
    AdvanceUntil(']');
    if (!Check(']')) {
      throw IniException(fmtstr("Invalid section header : {}", current_line), IniError::INVALID_SECTION);
    }

    // if (!Check('\n')) {
    //   throw IniException("Scriptable sections unimplemented", IniError::INVALID_SCRIPTABLE_SECTION);
    //   // else {
    //   //   // /// match '::' for scriptable sections
    //   //   // ConsumeUntil(':');
    //   //   // if (!Check(':')) {
    //   //   //   throw IniException("Invalid section header", IniError::FILE_PARSE_ERROR);
    //   //   // }
    //   //   // Consume();
    //   //   // if (!Check(':')) {
    //   //   //   throw IniException("Invalid section header", IniError::FILE_PARSE_ERROR);
    //   //   // }
    //   //   // Consume();

    //   //   // ParseScriptSection(current_line);
    //   // }
    // }

    ParseSection(current_line);

    ConsumeWhitespace();
  }

  void IniFileParser::HandleKey(bool allow_key_modifications) {
    /// save equals
    if (!AdvanceUntil('=')) {
      throw IniException(fmtstr("key value pair without value : {}", current_key.top()), IniError::KEY_WITHOUT_VALUE);
    }

    current_line += Advance();

    /// save whitespace
    if (AtEnd()) {
      throw IniException(fmtstr("key value pair without value : {}", current_key.top()), IniError::KEY_WITHOUT_VALUE);
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
      AdvanceUntil('\n');
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

  void IniFileParser::ConsumeWhitespace() {
    while (!AtEnd() && std::isspace(Peek())) {
      Consume();
    }
  }

  bool IniFileParser::AdvanceUntil(char c) {
    while (!AtEnd() && !Check(c)) {
      current_line += Advance();
    }
    return Check(c);
  }

  bool IniFileParser::AdvanceUntil(const std::span<const char>& chars) {
    while (!AtEnd() && !Check(chars)) {
      current_line += Advance();
    }
    return Check(chars);
  }

  bool IniFileParser::ConsumeUntil(char c) {
    while (!AtEnd() && !Check(c)) {
      current_line += Advance();
    }
    return Check(c);
  }

  bool IniFileParser::ConsumeUntil(const std::span<const char>& chars) {
    while (!AtEnd() && !Check(chars)) {
      current_line += Advance();
    }
    return Check(chars);
  }

  bool IniFileParser::Check(char c) {
    return Peek() == c;
  }

  bool IniFileParser::Check(const std::span<const char>& chars) {
    for (const auto& c : chars) {
      if (Check(c)) {
        return true;
      }
    }
    return false;
  }

  bool IniFileParser::Match(char c) {
    bool match = Check(c);
    if (match) {
      current_line += Advance();
    }
    return match;
  }

  bool IniFileParser::Match(const std::span<const char>& chars) {
    for (const auto& c : chars) {
      if (Match(c)) {
        return true;
      }
    }
    return false;
  }

}  // namespace other
