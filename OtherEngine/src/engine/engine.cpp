/**
 * \file engine/engine.cpp
 */
#include "engine/engine.hpp"

#include <optional>

#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "input/io.hpp"
#include "parsing/ini_parser.hpp"

#include "engine/engine_state_machine.hpp"

namespace other {

  Engine::Engine() {}

  Engine::Engine(const CmdLine& cmdline, Opt<std::string> main_thread_name)
      : cmd_line(cmdline) {
    exit_code = LoadConfig();

    Logger::Open(config);
    if (main_thread_name.has_value()) {
      Logger::Instance()->RegisterThread(main_thread_name.value());
    } else {
      Logger::Instance()->RegisterThread("Other-Engine-Main-Thread");
    }

    Filesystem::Initialize(cmdline, config);
    IO::Initialize();
    EventQueue::Initialize(config);

    state = NewRef<EngineStateMachine>(this);
    OE_ASSERT(state != nullptr, "Failed to create Engine State Machine");
  }

  Engine::~Engine() {
    state = nullptr;
    EventQueue::Shutdown();
    IO::Shutdown();
    Logger::Shutdown();
  }

  void Engine::Start() {
    exit_code = std::nullopt;
    delta.Start();

    EventQueue::RegisterEventDispatcher<ShutdownEvent>(
      "Other-Engine--Shutdown",
      { std::bind_front(&Engine::HandleShutdownEvent, this) }
    );
  }

  void Engine::Step() {
    AppState::OnEngineTick(delta.Get());

    while (!event_queue.empty()) {
      state->HandleEvent(event_queue.front());
      event_queue.pop();
    }

    if (state->IsFinished()) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      exit_code = AppState::exit_code.value();
    } else {
      state->Step();
    }
  }

  void Engine::Stop() {
    /// step once more to ensure all systems are shutdown
    Step();
  }

  bool Engine::IsRunning() const {
    return exit_code.has_value();
  }

  void Engine::EngineEvent(EngineStateEvent event) {
    OE_DEBUG("Engine Event : {}", event);
    event_queue.push(event);
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
  // ExitCode Engine::ProcessExitCode(ExitCode code) {
  //   switch (code) {
  //     case ExitCode::FAILURE:
  //       OE_CRITICAL("Application RUN failure");
  //       return code;

  //     case ExitCode::SUCCESS:
  //       return code;

  //     default:
  //       return ExitCode::FAILURE;
  //   }
  // }

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

  bool Engine::HandleShutdownEvent(ShutdownEvent& event) {
    OE_DEBUG("Engine Shutdown Event : {}", event.exit_code);
    AppState::exit_code = event.exit_code;
    EngineEvent(EngineStateEvent::ENGINE_SHUTDOWN);
    return false;
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
