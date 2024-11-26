/**
 * \file parsing\cmd_line_parser.hpp
 **/
#ifndef CMD_LINE_PARSER_HPP
#define CMD_LINE_PARSER_HPP

#include <array>
#include <map>
#include <string>
#include <vector>

#include "core/defines.hpp"

namespace other {

  struct RawArg {
    std::string_view sflag;
    std::string_view lflag;
    std::string_view description;

    bool has_args;
    uint32_t num_args;
    uint64_t hash = FNV(lflag);

    constexpr RawArg(std::string_view sflag, std::string_view lflag, std::string_view description, bool has_args, uint32_t num_args = 0) noexcept
        : sflag(sflag), lflag(lflag), description(description), has_args(has_args), num_args(num_args) {}
  };

  constexpr static size_t kNumArgs = 7;
  constexpr static std::array<RawArg, kNumArgs> kRawArgs = {
    RawArg("-h", "--help", "Prints this help message", false),
    RawArg("-v", "--version", "Prints the version of the program", false),
    RawArg("-p", "--project", "Specifies the project file to use", true, 1),
    RawArg("-cd", "--cwd", "Sets the current working directory", true, 1),
    RawArg("-e", "--editor", "Specifies the editor is to be opened and not the project", false),
    RawArg("-oe", "--other-environment", "Opens dynamic other engine environment terminal", false),
    RawArg("-t", "--testing", "If linked against OtherTestEngine library then toggles on internal test features, else it does nothing", false),
  };

  struct Arg {
    uint64_t hash = 0;
    std::string_view flag;
    std::vector<std::string> args;

    Arg() = default;
    Arg(std::string_view flag, const std::vector<std::string>& args)
        : hash(FNV(flag)), flag(flag), args(args) {}
    Arg(uint64_t hash, std::string_view flag, const std::vector<std::string>& args)
        : hash(hash), flag(flag), args(args) {}
  };

  class CmdLine {
   public:
    CmdLine() = default;
    CmdLine(int argc, char* argv[]);
    CmdLine(const std::vector<Arg>& args);

    void SetFlag(const std::string_view flag, const std::vector<std::string>& flag_args);

    bool HasFlag(const std::string_view flag) const;
    Opt<Arg> GetArg(const std::string_view flag) const;

    std::vector<std::pair<uint64_t, Arg>> GetArgs() const;

    struct RawCommandLine {
      int argc;
      char** argv;
    } raw_cmd_line;

   private:
    std::string program_name;
    std::vector<std::string> raw_args;
    std::map<uint64_t, Arg> args;

    void ProcessArgs() {}
  };

}  // namespace other

#endif  // !CMD_LINE_PARSER_HPP
