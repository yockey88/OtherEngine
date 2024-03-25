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

      std::map<uint64_t , Arg> Parse() {
        ParseArgs();
        return args;
      }

    private:
      int argc;
      const std::vector<std::string>& argv;

      size_t i = 0;
      std::map<uint64_t , Arg> args;

      void ParseArgs() {
        while (!AtEnd()) {
          const RawArg* ra = Peek();

          if (ra != nullptr) {
            Arg arg { 
              .hash = ra->hash ,
              .flag = ra->lflag  ,
              .args = {}
            };

            if (ra->has_args) {
              Advance();
              while (!AtEnd() && argv[i][0] != '-') {
                arg.args.push_back(argv[i]);
                Advance();
              }
            }

            args.insert({ arg.hash , arg });
          } else {
            println("Unknown flag: ", argv[i]);
          }

          Advance();
        }
      }

      bool AtEnd() const { return i >= argc; }
      const RawArg* Peek() const {
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
      void Advance() { i++; }
  };

} // namespace <anonymous>

  CmdLine::CmdLine(int argc , char* argv[]) {
    program_name = argv[0];

    raw_args.reserve(argc - 1);
    raw_args.insert(raw_args.end() , argv + 1 , argv + argc);

    CmdLineParser parser{ raw_args };
    args = parser.Parse();
  }

  bool CmdLine::HasFlag(const std::string& flag) const {
    uint64_t hash = FNV(flag);
    if (args.contains(hash)) {
      return true;
    } else {
      return false;
    }
  }

  Opt<Arg> CmdLine::GetArg(const std::string& flag) const {
    uint64_t hash = FNV(flag);
    if (args.contains(hash)) {
      return args.at(hash);
    } else {
      return std::nullopt;
    }
  }

} // namespace other
