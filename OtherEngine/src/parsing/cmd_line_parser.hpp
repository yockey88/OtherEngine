/**
 * \file parsing\cmd_line_parser.hpp
 **/
#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include <string>
#include <vector>
#include <array>
#include <map>

#include "core/defines.hpp"

namespace other {

  struct RawArg {
    std::string_view sflag;
    std::string_view lflag;
    std::string_view description;

    bool has_args;
    uint32_t num_args;
    uint64_t hash = FNV(lflag);

    constexpr RawArg(std::string_view sflag, std::string_view lflag, std::string_view description, bool has_args , uint32_t num_args = 0) noexcept
      : sflag(sflag), lflag(lflag), description(description), has_args(has_args) , num_args(num_args) {}
  };
  
  constexpr static size_t kNumArgs = 5; 
  constexpr static std::array<RawArg, kNumArgs> kRawArgs = {
    RawArg("-h", "--help", "Prints this help message", false),
    RawArg("-v", "--version", "Prints the version of the program", false),
    RawArg("-p", "--project", "Specifies the project file to use", true , 1),
    RawArg("-cd", "--cwd", "Sets the current working directory", true , 1) ,
    RawArg("-c", "--editor" , "Specifies the editor is to be opened and not the project", false)
  };

  struct Arg {
    uint64_t hash = 0;
    std::string_view flag;

    std::vector<std::string> args;
  };

  class CmdLine {
    public:
      CmdLine() = default;
      CmdLine(int argc , char* argv[]);

      bool HasFlag(const std::string& flag) const;
      Opt<Arg> GetArg(const std::string& flag) const;

    private:
      std::string program_name;
      std::vector<std::string> raw_args;
      std::map<uint64_t , Arg> args;

      void ProcessArgs() {}
  };

} // namespace other

#endif // !CMD_LINE_PARSER_HPP
