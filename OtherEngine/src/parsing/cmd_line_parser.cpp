/**
 * \file parsing\cmd_line_parser.cpp
 **/
#include "parsing\cmd_line_parser.hpp"

namespace other {
namespace {

  class CmdLineParser {
    public:
      CmdLineParser(const std::vector<std::string>& argv) 
        : argc(argv.size()) , argv(argv) {}

      std::map<uint64_t , Arg> Parse();

    private:
      int argc;
      const std::vector<std::string>& argv;

      size_t i = 0;
      std::map<uint64_t , Arg> args;

      void ParseArgs();
      bool AtEnd() const;
      const std::string PeekRaw() const;
      const RawArg* Peek() const;
      void Advance();
  };

} // namespace <anonymous>

  CmdLine::CmdLine(int argc , char* argv[]) {
    program_name = argv[0];

    raw_args.reserve(argc - 1);
    raw_args.insert(raw_args.end() , argv + 1 , argv + argc);

    CmdLineParser parser{ raw_args };
    args = parser.Parse();
  }

  void CmdLine::SetFlag(const std::string_view flag , const std::vector<std::string>& flag_args) {
    auto ra = std::ranges::find_if(kRawArgs, [&](const RawArg& arg) {
      return arg.sflag == flag || arg.lflag == flag;
    });

    if (ra == kRawArgs.end()) {
      return;
    }

    Arg arg = { ra->hash , ra->lflag , flag_args };
    const auto& [itr , inserted] = args.insert({ ra->hash , arg }); 
    if (!inserted) {
      args[ra->hash] = arg; 
    }
  }

  bool CmdLine::HasFlag(const std::string_view flag) const {
    uint64_t hash = FNV(flag);
    if (args.contains(hash)) {
      return true;
    } else {
      return false;
    }
  }

  Opt<Arg> CmdLine::GetArg(const std::string_view flag) const {
    uint64_t hash = FNV(flag);
    if (args.contains(hash)) {
      return args.at(hash);
    } else {
      return std::nullopt;
    }
  }

namespace {

  std::map<uint64_t , Arg> CmdLineParser::Parse() {
    ParseArgs();
    return args;
  }

  void CmdLineParser::ParseArgs() {
    while (!AtEnd()) {
      const RawArg* ra = Peek();

      if (ra != nullptr) {
        Arg arg { 
          .hash = ra->hash ,
          .flag = ra->lflag  ,
          .args = {}
        };

        if (ra->has_args) {
          for (uint32_t j = 0; j < ra->num_args; ++j) {
            Advance();
            if (AtEnd()) {
              println("Expected {} arguments for flag {}" , ra->num_args , ra->lflag);
              break;
            }

            arg.args.push_back(PeekRaw());
          }
        }

        args.insert({ arg.hash , arg });
      } else {
        println("Unknown flag: ", PeekRaw());
      }

      Advance();
    }

    for (const auto& [hash , arg] : args) {
      println("Flag: {}" , arg.flag);
      for (const auto& a : arg.args) {
        println("  Arg: {}" , a);
      }
    }
  }

  bool CmdLineParser::AtEnd() const { 
    return i >= argc; 
  }

  const std::string CmdLineParser::PeekRaw() const {
    if (AtEnd()) {
      return "";
    } else {
      return argv[i];
    }
  }

  const RawArg* CmdLineParser::Peek() const {
    if (AtEnd()) {
      return nullptr;
    } else {
      auto ra = std::ranges::find_if(kRawArgs, [&](const RawArg& arg) {
        return arg.sflag == argv[i] || arg.lflag == argv[i];
      });

      if (ra != kRawArgs.end()) {
        return &(*ra);
      } else {
        return nullptr;
      }
    }
  }

  void CmdLineParser::Advance() { 
    i++; 
  }

} // anonymous namespace
} // namespace other
