/**
 * \file parsing\ini_parser.hpp
*/
#ifndef OTHER_ENGINE_INI_PARSER_HPP
#define OTHER_ENGINE_INI_PARSER_HPP

#include <string>
#include <optional>

#include "core/defines.hpp"
#include "core/config.hpp"
#include "core/errors.hpp"

namespace other {

/// FIXME: why did I make this???? better way?
#define READ_INI_INTO(name , table , path) \
    try { \
      Path p = Filesystem::FindCoreFile(Path(path)); \
      IniFileParser parser(p.string()); \
      table = parser.Parse(); \
      OE_DEBUG("loaded " #name " config from {}" , p.string()); \
    } catch (const IniException& e) { \
      OE_ERROR("Failed to parse " #name " configuration file : {}", e.what()); \
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE); \
      return; \
    } catch (...) { \
      OE_ERROR("Caught unknown exception parsing " #name " configuration file"); \
      EventQueue::PushEvent<ShutdownEvent>(ExitCode::FAILURE); \
      return; \
    }

  class IniFileParser {
    public:
      IniFileParser(const std::string& file_path) 
        : file_path(file_path) {};
      ~IniFileParser() = default;

      ConfigTable Parse();
  
    private:
      std::string file_path;

      Opt<std::string> current_section = "";
      Opt<std::string> current_key = "";

      size_t index = 0;

      ConfigTable table;
      bool in_string = false;

      void Trim(std::string& str);
      void TrimQuotes(std::string& str);

      void ParseSection(const std::string& line);
      void ParseKeyValue(const std::string& line);
      void ParseValueList(const std::string& list);
  };

} // namespace other

#endif // !OTHER_ENGINE_INI_PARSER_HPP
