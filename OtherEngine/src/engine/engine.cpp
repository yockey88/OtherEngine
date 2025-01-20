/**
 * \file engine/engine.cpp
 */
#include "engine/engine.hpp"

#include <optional>

#include "core/config.hpp"
#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/ref.hpp"
#include "engine/engine_state_machine.hpp"

#include "application/app_state.hpp"
#include "asset/asset_database.hpp"
#include "asset/asset_manager.hpp"
#include "event/event_queue.hpp"
#include "input/io.hpp"
#include "parsing/ini_parser.hpp"

#include "memory/arena.hpp"

namespace other {

  Engine::Engine() {}

  Engine::Engine(const CmdLine& cmdline, std::string main_thread_name)
      : cmd_line(cmdline) {
    exit_code = LoadConfig();

    Logger::Open(config);
    Logger::Instance()->RegisterThread(main_thread_name);

    AssetDatabase::Initialize();
    Filesystem::Initialize(cmdline, config);
    IO::Initialize();
    EventQueue::Initialize(config);

    state = CreateStateMachine();
    OE_ASSERT(state != nullptr, "Failed to create Engine State Machine");

    exit_code = std::nullopt;
  }

  Engine::Engine(const ConfigTable& config, const CmdLine& cmd_line, std::string main_thread_name)
      : cmd_line(cmd_line), config(config) {
    Logger::Open(config);
    Logger::Instance()->RegisterThread(main_thread_name);

    Filesystem::Initialize(cmd_line, config);
    IO::Initialize();
    EventQueue::Initialize(config);

    state = CreateStateMachine();
    OE_ASSERT(state != nullptr, "Failed to create Engine State Machine");

    exit_code = std::nullopt;
  }

  Engine::~Engine() {
    state = nullptr;
    EventQueue::Shutdown();
    IO::Shutdown();
    Logger::Shutdown();
    AssetDatabase::Shutdown();
    AssetManager::Cleanup();
    Filesystem::Shutdown();

    if (detail::NumberOfLivingReferences() > 0) {
      println("Engine shutdown with {} living references", detail::NumberOfLivingReferences());
    }
  }

  void Engine::Run() {
    PROFILE_SECTION("Engine--Run");

    Start();
    OE_INFO("Running");
    do {
      Step();
    } while (!exit_code.has_value());

    OE_ASSERT(exit_code.has_value(), "Driver did not set exit code");
    Stop();
  }

  void Engine::Start() {
    delta.Start();
    EventQueue::RegisterEventDispatcher<ShutdownEvent>(
      "Other-Engine--Shutdown",
      { std::bind_front(&Engine::HandleShutdownEvent, this) }
    );
  }

  bool Engine::IsRunning() const {
    return exit_code.has_value();
  }

  void Engine::EngineEvent(EngineStateEvent event) {
    OE_DEBUG("Engine Event : {}", event);
    event_queue.push(event);
  }

  void Engine::Step() {
    PROFILE_SECTION("Engine--Step");
    ADD_MARK;

    dt = delta.Get();
    AppState::OnEngineTick(dt);

    if (!event_queue.empty()) {
      PROFILE_SECTION("Engine--state:HandleEvent");
      state->HandleEvent(event_queue.front());
      event_queue.pop();
    }

    if (state->IsFinished()) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      exit_code = AppState::exit_code.value();
    } else if (state->IsError()) {
      /// TODO: handle error state
    } else {
      state->Step();
    }
  }

  void Engine::Stop() {
    OE_ASSERT(state->IsFinished(), "Engine shutdown state corrupted");
    OE_ASSERT(exit_code.has_value(), "Engine did not set exit code");
    EventQueue::UnregisterEventDispatcher("Other-Engine--Shutdown");

    OE_INFO("Engine Exit : {}", exit_code.value());
  }

  Ref<EngineStateMachine> Engine::CreateStateMachine() {
    return NewRef<EngineStateMachine>(this);
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
      return ini_file;
    }

    println("FAILED to find configuration file for project in current directory!");
    println(" > defaulting to core configuration");

    /// if no .other file in current directory, use launcher file
    Path engine_core = Filesystem::GetEngineCoreDir();
    ini_file = engine_core / "OtherEngine" / "editor.other";
    if (!Filesystem::PathExists(ini_file.value())) {
      /// this means that engine core dir was not set correctly during build/install process
      println("Other Engine default configuration file not found [CORRUPT INSTALLATION]");
      return std::nullopt;
    }

    println("Opening Project Manager");
    println(" > {}", ini_file.value());
    return ini_file;
  }

  ExitCode Engine::LoadConfig() {
    println("Loading OtherEngine configuration");
    auto ini_file = FindConfigFile();
    if (!ini_file.has_value()) {
      println("Failed to find configuration file");
      return ExitCode::NO_CONFIG_FILE;
    }

    println(" > Using configuration : {}", config_path);
    config_path = ini_file.value().string();

    try {
      IniFileParser parser{ config_path };
      config = parser.Parse();
    } catch (IniException& e) {
      println("Failed to parse configuration file : {}", e.what());
      return ExitCode::CONFIG_PARSE_FAILURE;
    }

    std::string config_table_str = config.TableString();
#ifdef OTHER_DEBUG_BUILD
    println("project table :\n{}", config_table_str);
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
