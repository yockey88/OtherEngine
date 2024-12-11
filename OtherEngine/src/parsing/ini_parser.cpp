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
    stream = std::istringstream(contents);

    do {
      MainParseLoop();
    } while (index < contents.size());

    ConfigTable res = table;
    Reset();
    return res;
  }

  void IniFileParser::MainParseLoop() {
    switch (Peek()) {
      case '@':
        HandleObject();
        break;

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
        HandleIdentifier();
        break;
    }

    ++index;
    current_line.clear();
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

  void IniFileParser::ParseObjectSection(const std::string& type) {
    if (!Check('[')) {
      throw IniException(fmtstr("Invalid object : {}", type), IniError::INVALID_SECTION);
    }
    Consume();

    AdvanceUntil(']');
    if (AtEnd()) {
      throw IniException(fmtstr("Unclosed object section : {}", type), IniError::UNCLOSED_SECTION);
    }
    Consume();

    std::string object_name = current_line;
    Trim(object_name);
    if (object_name.empty()) {
      throw IniException(fmtstr("Invalid object : {}", type), IniError::INVALID_SECTION);
    }
    current_line.clear();

    ConsumeUntil({ '{', '=' });
    if (Check('=')) {
      ConsumeUntil('{');
    }

    std::stack<char> stack;
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

    std::string body = current_line;
    Trim(body);
    current_line.clear();

    if (type == "pipeline") {
      table.AddPipeline(object_name, body);
    } else if (type == "renderpass") {
      table.AddRenderPass(object_name, body);
    } else if (type == "framebuffer") {
      table.AddFramebufferSpec(object_name, body);
      std::cout << "Adding framebuffer spec : " << object_name << " :\n"
                << body << std::endl;
    } else if (type == "vertex-layout") {
      table.AddVertexLayout(object_name, body);
    } else if (type == "uniform") {
      table.AddUniform(object_name, body);
    } else {
      throw IniException(fmtstr("Invalid object section header : {} {}", type, object_name), IniError::INVALID_SECTION);
    }
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

    // Ref<Parser<void>> skip_comment = SkipUntil('\n');
    // Ref<Parser<void>> skip_newline = Skip('\n');
    // Ref<Parser<void>> skip_any = SkipAny();

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

  void IniFileParser::HandleIdentifier() {
    ConsumeWhitespace();
    AdvanceUntil([](char c) { return std::isspace(c) || c == '\n'; });
    if (current_line.empty()) {
      return;
    }

    std::string identifier = current_line;

    if (identifier == "function") {
      current_line.clear();
      HandleScript(identifier);
    } else {
      AdvanceUntil('=');
      current_line += Advance();
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
      ParseKeyValue(current_line, true);
    }
  }

  void IniFileParser::HandleSection() {
    AdvanceUntil(']');
    if (!Check(']')) {
      throw IniException(fmtstr("Invalid section header : {}", current_line), IniError::INVALID_SECTION);
    }
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

  void IniFileParser::HandleObject() {
    if (!Check('@')) {
      throw IniException(fmtstr("Invalid object section header : {}", current_line), IniError::INVALID_SECTION);
    }
    Consume();

    AdvanceUntil([&](char c) { return std::isspace(c) || c == '\n'; });
    if (current_line.empty()) {
      throw IniException(fmtstr("Invalid object section header : {}", current_line), IniError::INVALID_SECTION);
    }
    ConsumeWhitespace();

    std::string type = current_line;
    Trim(type);
    if (type.empty()) {
      throw IniException(fmtstr("Invalid object section header : {}", current_line), IniError::INVALID_SECTION);
    }
    current_line.clear();

    ParseObjectSection(type);
  }

  void IniFileParser::HandleScript(const std::string& callable) {
    ConsumeWhitespace();

    AdvanceUntil([](char c) { return std::isspace(c) || c == '\n'; });
    if (current_line.empty()) {
      return;
    }

    std::string func_name = current_line;
    Trim(func_name);
    current_line.clear();

    ConsumeWhitespace();
    if (!Check('{')) {
      throw IniException(fmtstr("Expected '{' after function name in scriptable section header : {}", func_name), IniError::INVALID_SECTION);
    }

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

    std::string body = current_line;
    Trim(body);
    current_line = "";

    ConfigTable::UnparsedScriptSection scriptable{
      .type = ValueType::EMPTY_TYPE,
      .source = body,
    };
    table.AddScriptSection(func_name, scriptable);
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
      stream.ignore();
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

  bool IniFileParser::AdvanceUntil(const std::vector<char>& chars) {
    while (!AtEnd() && !Check(chars)) {
      current_line += Advance();
    }
    return Check(chars);
  }

  bool IniFileParser::ConsumeUntil(char c) {
    while (!AtEnd() && !Check(c)) {
      Consume();
    }
    return Check(c);
  }

  bool IniFileParser::ConsumeUntil(const std::vector<char>& chars) {
    while (!AtEnd() && !Check(chars)) {
      Consume();
    }
    return Check(chars);
  }

  bool IniFileParser::Check(char c) {
    return Peek() == c;
  }

  bool IniFileParser::Check(const std::vector<char>& chars) {
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

  bool IniFileParser::Match(const std::vector<char>& chars) {
    for (const auto& c : chars) {
      if (Match(c)) {
        return true;
      }
    }
    return false;
  }

  template <typename Fn>
    requires requires(Fn f) {
      { f(std::declval<char>()) } -> std::convertible_to<bool>;
    }
  bool IniFileParser::AdvanceUntil(Fn&& f) {
    while (!AtEnd() && !f(Peek())) {
      current_line += Advance();
    }
    return f(Peek());
  }

}  // namespace other
