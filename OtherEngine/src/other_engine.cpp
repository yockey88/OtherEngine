/**
 * \file other_engine.cpp
 **/
#include "other_engine.hpp"

#include <filesystem>

#include "core/defines.hpp"
#include "core/logger.hpp"
#include "core/errors.hpp"
#include "core/engine.hpp"
#include "core/config.hpp"
#include "parsing/cmd_line_parser.hpp"
#include "parsing/ini_parser.hpp"
#include "project/project.hpp"

#include "event/event_queue.hpp"
#include "input/io.hpp"
#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

namespace other { 

  CmdLine OE::cmd_line;
  ConfigTable OE::current_config;

  Path OE::config_path;

  ExitCode OE::Main(int argc , char* argv[]) {
#ifdef OE_DEBUG_BUILD
    println("OtherEngine Debug Build");
#endif 
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

    auto set_cwd = cmd_line.GetArg("--cwd");
    if (set_cwd.has_value()) {
      if (set_cwd.value().args.size() != 1) {
        other::println("Invalid number of arguments for --cwd");
        return ExitCode::FAILURE;
      }

      if (!std::filesystem::exists(set_cwd.value().args[0])) {
        other::println("Invalid path for --cwd");
        return ExitCode::FAILURE;
      }
      std::filesystem::current_path(set_cwd.value().args[0]);
    }

    println("Starting Other Engine in {}" , std::filesystem::current_path().string());
    if (!LoadConfig()) {
      other::println("Failed to load configuration");
      return ExitCode::FAILURE;
    }

    ExitCode ec = ExitCode::SUCCESS;

    OE driver;
    try {
      CoreInit();

      /* main engine entry point */ {
        ExitCode run_code = driver.Run();
        if (run_code != ExitCode::SUCCESS) {
          std::cerr << "Failed to run application" << std::endl;
        }
      }
    } catch (const std::exception& e) {
      OE_CRITICAL("Fatal Error Caught : {}", e.what());
      ec = ExitCode::FAILURE;
    } catch (...) {
      OE_CRITICAL("Caught unknow error!");
      ec = ExitCode::FAILURE;
    }

    if (!driver.full_shutdown) {
      driver.Shutdown();
    }
    if (!driver.engine_unloaded) {
      driver.engine.UnloadApp();
    }
    CoreShutdown();
    return ec;
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

  std::string OE::FindConfigFile() {
    if (Project::HasQueuedProject()) {
      auto qpath = Project::GetQueuedProjectPath(); 
      println("Using queued project : {}", qpath);

      if (!std::filesystem::exists(qpath)) {
        other::println("Queued project not found : {}", qpath);
        return "";
      }

      return qpath;
    }

    auto cfg = cmd_line.GetArg("--project");

    std::string ini_file = "";
    if (cfg.has_value()) {
      if (cfg.value().args.size() != 1) {
        other::println("Invalid number of arguments for --project");
        return "";
      }

      if (!std::filesystem::exists(cfg.value().args[0])) {
        other::println("Configuration file not found : {} , using default", cfg.value().args[0]);
      } else {
        ini_file = cfg.value().args[0];
      }
    } 

    if (ini_file.empty()) {
      config_path = std::filesystem::current_path() / "OtherEngine-Launcher" / "launcher.other";
      if (!std::filesystem::exists(config_path)) {
        other::println("Default configuration file not found");
        return "";
      }
      ini_file = config_path.string();

      // if not found yet search for any other .other file in current directory 
      if (ini_file.empty()) {
        for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::current_path())) {
          if (entry.path().extension() == ".other") {
            ini_file = entry.path().string();
          }
        }
      }
    }

    return ini_file;
  }
  
  bool OE::LoadConfig() {
    auto ini_file = FindConfigFile();
    if (ini_file.empty()) {
      other::println("Failed to find configuration file");
      return false;
    }

    println("Using configuration : {}", ini_file);

    config_path = ini_file;

    try {
      IniFileParser parser{ ini_file };
      current_config = parser.Parse();
    } catch (IniException& e) {
      println("Failed to parse configuration file : {}", e.what());
      return false;
    }

    auto config_table_str = current_config.TableString();

    return true;
  }

  void OE::CoreInit() {
    // open logger
    Logger::Open(current_config);
    Logger::Instance()->RegisterThread("OE-Thread");

    // init allocators
  }

  void OE::Launch() {
    IO::Initialize();
    EventQueue::Initialize(current_config);

    Renderer::Initialize(current_config);
    CHECKGL();

    UI::Initialize(current_config , Renderer::GetWindow());
    ScriptEngine::Initialize(current_config);
  }

  ExitCode OE::Run() {
    bool should_quit = false;

    engine = Engine::Create(cmd_line, current_config, config_path.string());

    do {
      engine_unloaded = false;
      full_shutdown = false;
       
      /// launch the application and load its main data
      {
        auto app = NewApp(&engine);
        engine.LoadApp(app);
      }
      
      /// launch the engine, initializing subsystems
      Launch();

      /// run the app
      engine.PushCoreLayer();
      engine.ActiveApp()->Run();

      auto exit_code = engine.exit_code.value();
      switch (exit_code) {
        case ExitCode::FAILURE:
          OE_CRITICAL("Application RUN failure");
          should_quit = true;
          break;
        case ExitCode::SUCCESS:
          should_quit = true;
          break;
        case ExitCode::LOAD_NEW_PROJECT:
        case ExitCode::RELOAD_PROJECT:
          /// Full Shutdown
          Shutdown();
          CoreShutdown();

          /// Reload configuration
          if (!LoadConfig()) {
            println("Failed to load configuration");
            return ExitCode::FAILURE;
          }

          /// begin again
          CoreInit();
          continue;
        default:
          break;
      }

      /// shutdown the engine, closing and offloading main subsystems
      Shutdown();
      full_shutdown = true;
      
      /// unload the app, offloading core data and pushing app out of scope to call destructor 
      engine.UnloadApp();
      engine_unloaded = true;
    } while (!should_quit);

    return engine.exit_code.value();
  }

  void OE::Shutdown() {
    ScriptEngine::Shutdown();

    UI::Shutdown();
    Renderer::Shutdown();
    EventQueue::Shutdown();
    IO::Shutdown();

    OE_INFO("Shutdown complete");
  }

  void OE::CoreShutdown() {
    // shutdown allocators

    // close logger
    OE_INFO("Core shutdown complete, closing logger");
    Logger::Shutdown();
  }

} // namespace other
