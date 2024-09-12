/**
 * \file other_engine.cpp
 **/
#include "other_engine.hpp"

#include <filesystem>

#include "core/defines.hpp"
#include "core/engine.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "event/event_queue.hpp"

namespace other { 
namespace {

  void Help();
  void Version();
  ExitCode ProcessArgs(const CmdLine& cmd_line);

} // anonymous namespace

  ExitCode Main(int argc , char* argv[]) {
#ifdef OE_DEBUG_BUILD
    println("OtherEngine Debug Build");
#endif // !OE_DEBUG_BUILD
  
    CmdLine cmd_line(argc , argv);
    ExitCode ec = ProcessArgs(cmd_line);
    if (ec != ExitCode::NO_EXIT) {
      return ec;
    }

    Engine driver(cmd_line);
    try {
      ec = driver.Run();
    } catch (...) {
      println("Unknown exception caught at top level : MAJOR ERROR");
      ec = ExitCode::FAILURE;
    }
    return ec;
  }

namespace {

  void Help() {
    println("[ Other Engine CLI ]");
    println("- Usage: other_engine.exe [options]");
    println("- Options:");
    for (const auto& arg : kRawArgs) {
      println("  {} , {} : {}", arg.sflag , arg.lflag , arg.description);
    }
  }

  void Version() {
    println("Other Engine v0.0.1");
  }

  ExitCode ProcessArgs(const CmdLine& cmd_line) {
    auto help = cmd_line.HasFlag("--help");
    auto version = cmd_line.HasFlag("--version");

    if (version) {
      Version();
    }

    if (help) {
      Help();
    }

    if (help || version) {
      return ExitCode::SUCCESS;
    }

    auto set_cwd = cmd_line.GetArg("--cwd");
    if (!set_cwd.has_value()) {
      return ExitCode::NO_EXIT;
    }

    if (set_cwd->args.size() != 1) {
      other::println("Invalid number of arguments for --cwd");
      return ExitCode::FAILURE;
    }

    if (!std::filesystem::exists(set_cwd->args[0])) {
      other::println("Invalid path for --cwd");
      return ExitCode::FAILURE;
    }

    std::filesystem::current_path(set_cwd->args[0]);
    return ExitCode::NO_EXIT;
  }

} // anonymous namespace
} // namespace other
