/**
 * \file other_engine.cpp
 **/
#include "other_engine.hpp"

#include <filesystem>

#include "core/defines.hpp"
#include "engine/engine.hpp"
#include "memory/arena.hpp"

#include "event/event_queue.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "steam/steam_manager.hpp"

namespace other {
  namespace {

    void Help();
    void Version();
    ExitCode ProcessArgs(const CmdLine& cmd_line);

  }  // anonymous namespace

  ExitCode Main(int argc, char* argv[]) {
    PROFILE_SECTION("OtherEngine--Main");

    CmdLine cmd_line(argc, argv);
    ExitCode ec = ProcessArgs(cmd_line);
#ifdef OE_DEBUG_BUILD
    println("OtherEngine Debug Build");
#endif  // !OE_DEBUG_BUILD
    if (ec != ExitCode::NO_EXIT) {
      return ec;
    }

    Arena::Initialize();
    Engine* driver = LoadDriver(cmd_line);
    OE_ASSERT(driver != nullptr, "Failed to load driver");
    if (driver->config.KeyExists("steam", "app-id")) {
      int32_t app_id = driver->config.GetVal<int32_t>("steam", "app-id").value_or(0);
      if (app_id != 0 && SteamManager::CheckForRestart(app_id)) {
        UnloadDriver(driver);
        Arena::Shutdown();
        return ExitCode::SUCCESS;
      }
    }

    try {
      OE_ASSERT(driver != nullptr, "Failed to load driver");
      driver->Run();
      ec = driver->exit_code.value();
    } catch (std::exception& e) {
      println("caught error at top level! error : {}", e.what());
      ec = ExitCode::FAILURE;
    } catch (...) {
      println("Unknown exception caught at top level : MAJOR ERROR");
      ec = ExitCode::FAILURE;
    }

    UnloadDriver(driver);
    Arena::Shutdown();

    return ec;
  }

  namespace {

    void Help() {
      println("[ Other Engine CLI ]");
      println("- Usage: other_engine.exe [options]");
      println("- Options:");
      for (const auto& arg : kRawArgs) {
        println("  {} , {} : {}", arg.sflag, arg.lflag, arg.description);
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

  }  // anonymous namespace
}  // namespace other
