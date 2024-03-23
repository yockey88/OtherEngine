/**
 * \file other_engine.cpp
 **/
#include "other_engine.hpp"

#include <filesystem>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/engine.hpp"
#include "event/event_queue.hpp"
#include "input/io.hpp"
#include "rendering/renderer.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"

int rendering_main(const other::ConfigTable& config);

namespace other { 

  CmdLine OE::cmd_line;
  ConfigTable OE::current_config;

  std::filesystem::path OE::config_path;

  ExitCode OE::Main(int argc , char* argv[]) {
    cmd_line = other::CmdLine(argc , argv);

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

    if (!LoadConfig()) {
      other::println("Failed to load configuration");
      return ExitCode::FAILURE;
    }

    try {
      CoreInit();

      {
        OE driver;
        ExitCode run_code = driver.Run();
        if (run_code != ExitCode::SUCCESS) {
          std::cerr << "Failed to run application" << std::endl;
        }
      }

      CoreShutdown();
    } catch (other::IniException& e) {
      OE_CRITICAL("{}", e.what());
      return ExitCode::FAILURE;
    }

    return ExitCode::SUCCESS;
  }
      
  void OE::Help() {
    println("[ Other Engine CLI ]");
    println("- Usage: other_engine.exe [options]");
    println("- Options:");
    for (const auto& arg : kRawArgs) {
      println("  {} , {} : {}", arg.sflag , arg.lflag , arg.description);
    }
  }

  void OE::Version() {
    println("Other Engine v0.0.1");
  }

  void OE::CoreInit() {
    auto log_level = current_config.Get("LOG-LEVEL");
    // open logger
    Logger::Open(current_config);
    Logger::Instance()->RegisterThread("OE-Thread");

    // init allocators
  }

  bool OE::LoadConfig() {
    auto cfg = cmd_line.GetArg("--ini");

    std::string ini_file = "";
    if (cfg.has_value()) {
      if (cfg.value().args.size() != 1) {
        other::println("Invalid number of arguments for --ini");
        return false;
      }

      if (!std::filesystem::exists(cfg.value().args[0])) {
        ini_file = "";
      } else {
        ini_file = cfg.value().args[0];
      }
    } 

    if (ini_file.empty()) {
      config_path = std::filesystem::current_path() / "OtherEngine" / "resources" / "default_config.other";
      if (!std::filesystem::exists(config_path)) {
        other::println("Default configuration file not found");
        return false;
      }
      ini_file = config_path.string();
    } 

    other::println("Using configuration : {}", ini_file);
    other::IniFileParser parser{ ini_file };
    current_config = parser.Parse();
    return true;
  }

  void OE::Launch() {
    engine = Engine(current_config);

    IO::Initialize();
    EventQueue::Initialize();
    Renderer::Initialize(current_config);
  }

  ExitCode OE::Run() {
    Launch();

    bool should_quit = false;
    do {
      {
        auto app = CreateApp(&engine , current_config);
        engine.LoadApp(app);
      }

      engine.ActiveApp()->Run();
      engine.UnloadApp();

      auto exit_code = engine.exit_code.value();
      switch (exit_code) {
        case ExitCode::FAILURE:
          OE_CRITICAL("Application RUN failure");
        case ExitCode::SUCCESS:
          should_quit = true;
          break;
        case ExitCode::RELOAD_PROJECT:
          println("RELOADING PROJECT UNIMPLEMENTED | PLEASE RESTART WITH CORRECT CONFIGURATION");
          should_quit = true;
          break;
        default:
          break;
      }
    } while (!should_quit);


    Renderer::Shutdown();
    EventQueue::Shutdown();

    // force call of destructors
    return engine.exit_code.value();
  }

  void OE::CoreShutdown() {
    // shutdown allocators
    // close logger
    Logger::Shutdown();
  }

} // namespace other
