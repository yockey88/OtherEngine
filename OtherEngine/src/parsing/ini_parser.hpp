/**
 * \file parsing\ini_parser.hpp
 */
#ifndef OTHER_ENGINE_INI_PARSER_HPP
#define OTHER_ENGINE_INI_PARSER_HPP

#include <span>
#include <stack>
#include <string>

#include "core/config.hpp"
#include "core/defines.hpp"


namespace other {

/// FIXME: why did I make this????
#define READ_INI_INTO(name, table, path)                                       \
  try {                                                                        \
    Path p = Filesystem::FindCoreFile(Path(path));                             \
    IniFileParser parser(p.string());                                          \
    table = parser.Parse();                                                    \
    OE_DEBUG("loaded " #name " config from {}", p.string());                   \
  } catch (const IniException& e) {                                            \
    OE_ERROR("Failed to parse " #name " configuration file : {}", e.what());   \
    EventQueue::PushEvent<ShutdownEvent>({ ExitCode::FAILURE });               \
    return;                                                                    \
  } catch (...) {                                                              \
    OE_ERROR("Caught unknown exception parsing " #name " configuration file"); \
    EventQueue::PushEvent<ShutdownEvent>({ ExitCode::FAILURE });               \
    return;                                                                    \
  }

  class IniFileParser {
   public:
    IniFileParser(const std::string& file_path)
        : file_path(file_path){};
    ~IniFileParser() = default;

    ConfigTable Parse();

   private:
    std::string file_path;
    std::string contents;
    std::string current_line;

    Opt<std::string> current_section = "";
    std::stack<std::string> current_key = {};
    std::string full_current_key = "";

    size_t index = 0;

    ConfigTable table;
    bool in_string = false;

    void Trim(std::string& str);
    void TrimQuotes(std::string& str);

    std::string StripParens(const std::string& str);
    std::vector<std::string> SplitOn(const std::string& str, char c);
    std::pair<std::string, std::string> SplitOnFirst(const std::string& str, char c);

    void PushKey(const std::string& key);
    void PopKey();

    void ParseSection(const std::string& line);
    void ParseScriptSection(const std::string& line);
    void ParseKeyValue(const std::string& line, bool allow_key_modifications);
    void ParseKey(const std::string& key, bool allow_key_modifications);
    void ParseValue(const std::string& value, bool allow_key_modifications);
    void ParseValueList(const std::string& list, bool allow_key_modifications);

    std::string GetFullKey() const;

    void HandleComment();
    void HandleSection();
    void HandleKey(bool allow_key_modifications);

    bool AtEnd() const;
    char Peek() const;
    char Previous() const;

    char Advance();
    void Consume();
    bool AdvanceUntil(char c);
    bool AdvanceUntil(const std::span<const char>& chars);
    bool ConsumeUntil(char c);
    bool ConsumeUntil(const std::span<const char>& chars);

    bool Check(char c);
    bool Check(const std::span<const char>& chars);
    bool Match(char c);
    bool Match(const std::span<const char>& chars);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_INI_PARSER_HPP
