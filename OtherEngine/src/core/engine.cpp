/**engine.c
 * \file core/engine.cpp
 */
#include "core/engine.hpp"

#include <optional>

#include "core/config.hpp"
#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/engine_state.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "application/runtime_layer.hpp"
#include "event/event_queue.hpp"
#include "input/io.hpp"
#include "parsing/ini_parser.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

#include "editor/editor.hpp"
#include "editor/editor_console_sink.hpp"
#include "editor/editor_layer.hpp"
#include "editor/test_editor_layer.hpp"

namespace other {

  Engine::Engine() {}

  Engine::Engine(const CmdLine& cmdline)
      : cmd_line(cmdline) {
    EngineState::exit_code = LoadConfig();
  }

  ExitCode Engine::Run() {
#ifdef OE_DEBUG_BUILD
    println("Running Other Engine in {}", std::filesystem::current_path().string());
#endif  // OE_DEBUG_BUILD

    if (!EngineState::exit_code.has_value()) {
      println("Engine in invalid state for Engine::Run(). config never loaded");
      return ExitCode::FAILURE;
    }

    if (EngineState::exit_code.value() != ExitCode::NO_EXIT) {
      println("Failed to load configuration!");
      return ExitCode::FAILURE;
    }

    EngineState::exit_code = std::nullopt;
    ExitCode ec;

    /// TODO: initialize allocators

    Logger::Open(config);
    Logger::Instance()->RegisterThread("Other Engine Driver Thread");
    /// Initialize Sub-Systems
    ///   - logger
    ///   - component manager
    ///   -
    /// Load Components
    ///

    try {
      do {
        LoadApp();
        active_app->Run();
        UnloadApp();
      } while (!EngineState::exit_code.has_value());
      ec = EngineState::exit_code.value();
    } catch (const std::exception& e) {
      OE_CRITICAL("Fatal error caught (std::exception) : {}", e.what());
      ec = ExitCode::FAILURE;
    } catch (...) {
      OE_CRITICAL("Fatal error caught (UNKNOWN)");
      ec = ExitCode::FAILURE;
    }

    Logger::Shutdown();

    /// TODO: shutdown allocators

    return ec;
  }

  void Engine::LoadApp() {
#ifndef OTHERENGINE_DLL
    auto app = NewApp(cmd_line, config);
    OE_ASSERT(app != nullptr, "Attempting to load a null application (client implementation of other::NewApp(Engine*) is invalid)");

    println("Loading application");
    bool in_editor = cmd_line.HasFlag("--editor");

    if (in_editor) {
      auto editor_app = NewScope<Editor>(cmd_line, config /* , app */);
      active_app = std::move(editor_app);
    } else {
      active_app = std::move(app);
    }

    active_app->Load();
    AppState::Initialize(active_app.get(), active_app->layer_stack, active_app->scene_manager,
                         active_app->asset_handler, active_app->project_metadata);

    Launch();

    Ref<Layer> core_layer = nullptr;
    if (in_editor) {
      if (config.GetVal<bool>(kDebugSection, "TEST-EDITOR", false).value_or(false)) {
        core_layer = NewRef<TEditorLayer>(active_app.get(), active_app->config);
      } else {
        core_layer = NewRef<EditorLayer>(active_app.get(), active_app->config);
      }
      AppState::mode = EngineMode::DEBUG;
    } else {
      core_layer = NewRef<RuntimeLayer>(active_app.get(), active_app->config);
      AppState::mode = EngineMode::RUNTIME;
    }
    active_app->PushLayer(core_layer);

    if (in_editor) {
      Logger::Instance()->RegisterTarget({
        .target_name = "Editor-Console",
        .level = Logger::LevelFromLevel(Logger::Level::DEBUG),
        .log_format = "%v",
        .sink_factory = []() -> spdlog::sink_ptr {
          return NewStdRef<EditorConsoleSink>(10);
        },
      });
    }
#endif  // !OTHERENGINE_DLL
  }

  void Engine::UnloadApp() {
#ifndef OTHERENGINE_DLL
    Shutdown();

    OE_ASSERT(active_app != nullptr, "Attempting to unload a null application");

    active_app->Unload();
    active_app = nullptr;

    OE_DEBUG("Engine unloaded");
#endif  // !OTHERENGINE_DLL
  }

  void Engine::Launch() {
    IO::Initialize();
    EventQueue::Initialize(config);

    Renderer::Initialize(config);
    CHECKGL();

    UI::Initialize(config, Renderer::GetWindow());
    ScriptEngine::Initialize(config);

    PhysicsEngine::Initialize(config);
  }

  void Engine::Shutdown() {
    PhysicsEngine::Shutdown();

    ScriptEngine::Shutdown();

    UI::Shutdown();
    Renderer::Shutdown();
    EventQueue::Shutdown();
    IO::Shutdown();

    OE_INFO("Shutdown complete");
  }

  Opt<Path> Engine::FindConfigFile() {
    Opt<Path> cwd = std::nullopt;
    Opt<Path> ini_file = std::nullopt;

    ProcessSingleArg("--cwd", 1, [&cwd](Arg& arg) -> bool {
      auto path = arg.args[0];
      if (Filesystem::PathExists(path)) {
        cwd = path;
      }

      return cwd.has_value();
    });

    ProcessSingleArg("--project", 1, [&ini_file](Arg& arg) -> bool {
      auto path = arg.args[0];

      println("attempting to find config file : {}", path);
      if (Filesystem::PathExists(path)) {
        println(" > using configuration : {}", path);
        ini_file = path;
      } else {
        println(" > failed to find configuration file");
      }

      return ini_file.has_value();
    });

    if (!cwd.has_value()) {
      cwd = Filesystem::GetWorkingDirectory();
    }
    Path curr_dir = cwd.value();

    // if not on command line search for .other file in current directory
    if (!ini_file.has_value()) {
      println("searching {} for config", curr_dir);
      for (const auto& entry : std::filesystem::directory_iterator(curr_dir)) {
        if (entry.path().extension() == ".other") {
          ini_file = entry.path().string();
        }
      }
    }

    /// if no .other file in current directory, use launcher file
    if (!ini_file.has_value()) {
      Path engine_core = Filesystem::GetEngineCoreDir();
      ini_file = engine_core / "OtherEngine-Launcher" / "launcher.other";

      println("Defaulting to {}", ini_file.value());
      if (!Filesystem::PathExists(ini_file.value())) {
        /// this means that engine core dir was not set correctly during build/install process
        println("Other Engine Launcher configuration file not found [CORRUPT INSTALLATION]");
        ini_file = std::nullopt;
      }
    }

    return ini_file;
  }

  /// FIXME: switch missing cases!!! default triggered in most cases
  ExitCode Engine::ProcessExitCode(ExitCode code) {
    switch (code) {
      case ExitCode::FAILURE:
        OE_CRITICAL("Application RUN failure");
        return code;

      case ExitCode::SUCCESS:
        return code;

      default:
        return ExitCode::FAILURE;
    }
  }

  ExitCode Engine::LoadConfig() {
    auto ini_file = FindConfigFile();
    if (!ini_file.has_value()) {
      return ExitCode::NO_CONFIG_FILE;
    }

    config_path = ini_file.value().string();
    println("Using configuration : {}", config_path);

    try {
      IniFileParser parser{ config_path };
      config = parser.Parse();
    } catch (IniException& e) {
      println("Failed to parse configuration file : {}", e.what());
      return ExitCode::CONFIG_PARSE_FAILURE;
    }

    auto config_table_str = config.TableString();
#ifdef OE_DEBUG_BUILD
    println(config_table_str);
#endif  // OE_DEBUG_BUILD

    return ExitCode::NO_EXIT;
  }

  void Engine::ProcessSingleArg(const std::string_view lflag, uint32_t min_args, std::function<bool(Arg&)> processor) {
    auto arg = cmd_line.GetArg(lflag);

    if (!arg.has_value()) {
      return;
    }

    if (arg.value().args.size() < min_args) {
      println("Invalid number of arguments for {}", lflag);
      return;
    }

    if (processor == nullptr || !processor(arg.value())) {
      println("Failed to process arg : {}", lflag);
    }
  }

}  // namespace other
