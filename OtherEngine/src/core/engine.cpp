/**engine.c
 * \file core/engine.cpp
 */
#include "core/engine.hpp"

#include <optional>

#include "core/config.hpp"
#include "core/config_keys.hpp"
#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "application/app_state_machine.hpp"
#include "application/runtime_layer.hpp"
#include "event/core_events.hpp"
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
    exit_code = LoadConfig();
  }

  ExitCode Engine::Run() {
#ifdef OE_DEBUG_BUILD
    println("Running Other Engine in {}", std::filesystem::current_path().string());
#endif  // OE_DEBUG_BUILD

    if (!exit_code.has_value()) {
      println("Engine in invalid state for Engine::Run(). config never loaded");
      return ExitCode::FAILURE;
    }

    if (exit_code.value() != ExitCode::NO_EXIT) {
      println("Failed to load configuration!");
      return ExitCode::FAILURE;
    }

    exit_code = std::nullopt;
    ExitCode ec;

    /// TODO: allocators

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
        // active_app->Run();
        UnloadApp();
      } while (!exit_code.has_value());
      ec = exit_code.value();
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
    App* app = NewApp(cmd_line, config);
    OE_ASSERT(app != nullptr, "Attempting to load a null application (client implementation of other::NewApp(Engine*) is invalid)");

    println("Loading application");
    bool in_editor = cmd_line.HasFlag("--editor");
    App* active_app = nullptr;

    if (in_editor) {
      App* editor_app = new Editor(cmd_line, config /* , app */);
      active_app = editor_app;
    } else {
      active_app = std::move(app);
    }

    AppState::Initialize(cmd_line, config, active_app);

    Launch();
    if (in_editor) {
      PushCoreLayer();
      RegisterLoggers();
    }

    AppState::AppEvent(NewRef<ApplicationAttached>());
  }

  void Engine::UnloadApp() {
    Shutdown();
    AppState::Shutdown();

    OE_DEBUG("Engine unloaded");
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

  void Engine::Start() {
    exit_code = std::nullopt;
    delta.Start();
  }

  void Engine::Step() {
    float dt = delta.Get();
    AppState::OnEngineTick(dt);
    EventQueue::Poll();
  }

  void Engine::Stop() {
    /// stop command
    EventQueue::PushEvent<StopCommand>({});

    /// poll to enforce stop
    EventQueue::Poll();
    /// flush loop
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    /// stopped

    AppState::DetachApplication();
  }

  Opt<Path> Engine::FindConfigFile() {
    Opt<Path> ini_file = std::nullopt;
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

    if (ini_file.has_value()) {
      return ini_file;
    }

    Opt<Path> cwd = std::nullopt;
    ProcessSingleArg("--cwd", 1, [&cwd](Arg& arg) -> bool {
      auto path = arg.args[0];
      if (Filesystem::PathExists(path)) {
        cwd = path;
      }

      return cwd.has_value();
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

    if (ini_file.has_value()) {
      println("Using configuration : {}", ini_file.value());
      return ini_file;
    }

    /// if no .other file in current directory, use launcher file
    Path engine_core = Filesystem::GetEngineCoreDir();
    ini_file = engine_core / "OtherEngine-Launcher" / "launcher.other";
    if (!Filesystem::PathExists(ini_file.value())) {
      /// this means that engine core dir was not set correctly during build/install process
      println("Other Engine Launcher configuration file not found [CORRUPT INSTALLATION]");
      return std::nullopt;
    }

    println("Opening Project Manager");
    println(" > {}", ini_file.value());
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

  void Engine::PushCoreLayer() {
    Ref<Layer> core_layer = nullptr;

    App& active_app = AppState::AppHandle();
    bool in_editor = cmd_line.HasFlag("--editor");
    bool debug_editor = config.GetVal<bool>(kDebugSection, "EDITOR").value_or(false);

    if (in_editor || debug_editor) {
      if (debug_editor) {
        core_layer = NewRef<TEditorLayer>(&active_app, active_app.config);
      } else {
        core_layer = NewRef<EditorLayer>(&active_app, active_app.config);
      }
      AppState::mode = EngineMode::EDITOR;
    } else {
      core_layer = NewRef<RuntimeLayer>(&active_app, active_app.config);
      AppState::mode = EngineMode::RUNTIME;
    }

    AppState::PushLayer(core_layer);
  }

  void Engine::RegisterLoggers() {
    /// TODO: register client loggers (if any)

    Logger::Instance()->RegisterTarget({
      .target_name = "Editor-Console",
      .level = Logger::LevelFromLevel(Logger::Level::DEBUG),
      .log_format = "%v",
      .sink_factory = []() -> spdlog::sink_ptr {
        return NewStdRef<EditorConsoleSink>(10);
      },
    });
  }

  void Engine::ProcessSingleArg(const std::string_view lflag, uint32_t min_args, std::function<bool(Arg&)> processor) {
    auto arg = cmd_line.GetArg(lflag);
    if (!arg.has_value()) {
      return;
    }

    OE_ASSERT(arg.value().args.size() >= min_args, "Invalid number of arguments for {}", lflag);
    OE_ASSERT(processor != nullptr, "No processor for {}", lflag);

    if (!processor(arg.value())) {
      println("Failed to process arg : {}", lflag);
    }
  }

}  // namespace other
