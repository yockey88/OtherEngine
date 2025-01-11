/**
 * \file engine/engine_states.cpp
 **/
#include "engine/engine_states.hpp"

#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "engine/engine.hpp"

#include "application/app_state.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"

#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

#include "editor/editor_sink.hpp"
#include "editor/editor_states.hpp"

#ifdef OE_TESTING_ENVIRONMENT
  #include "testing_core/test_engine_states.hpp"
#endif  // OE_TESTING_ENVIRONMENT

namespace other {
  namespace {

    bool HandleWindowClosed(WindowClosed& event) {
      EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
      return false;
    }

    bool HandleDeleteFile(DeleteFileEvent& event) {
      OE_DEBUG("Deleting file : {}", event.handle);
      if (!Filesystem::RemoveFile(event.handle)) {
        OE_ERROR("Failed to delete file : {}", event.handle);
        return false;
      }
      return true;
    }

    bool HandleCreateFile(CreateFileEvent& event) {
      Ref<Directory> dir = Filesystem::GetDirectory(event.handle);
      if (dir == nullptr) {
        OE_ERROR("Failed to get directory to handle file creation : {}", event.handle);
        return false;
      }

      dir->Update();
      return true;
    }

    // bool HandleKeyPress(KeyPressed& event) {
    //   /// TODO: remove this, just for fast development iteration
    //   return HandleKeyEvent(event, Keyboard::Key::OE_ESCAPE, [&]() -> bool {
    //     EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
    //     return true;
    //   });
    // }

  }  // anonymous namespace

  Ref<EngineState> IdleState::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::EDITOR_START) {
      return NewRef<EngineShutdown>(engine);
    }

    return nullptr;
  }

  Ref<EngineState> ErrorState::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      return NewRef<EngineShutdown>(engine);
    }

    return nullptr;
  }

  // void ErrorState::OnAttach() {
  // }

  // void ErrorState::OnStep() {
  // }

  // void ErrorState::OnDetach() {
  // }

  Ref<EngineState> EngineLaunching::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::ENGINE_LOAD_FINISHED) {
      return NewRef<EngineIdle>(engine);
    }

    return nullptr;
  }

  void EngineLaunching::OnAttach() {
    EventQueue::RegisterEventDispatcher<WindowClosed>(
      "Other-Engine--WindowClosed",
      { &HandleWindowClosed }
    );
  }

  void EngineLaunching::OnStep() {
    /// TODO: implement actual engine launch logic,
    ///       - check if headless mode
    ///       - check configuration settings (server, client, editor, runtime, etc....)
    // if (engine->cmd_line.HasFlag("--headless")) {
    //   AppState::mode = EngineMode::HEADLESS;
    // }

    AppState::Initialize(engine);
    Renderer::Initialize(engine->config);
    UI::Initialize(engine->config, Renderer::GetWindow());
    ScriptEngine::Initialize(engine->config);

    /// MAYBE: indicate what mode the engine is running using this event (headless_load_finished, server_load_finished, etc...)
    engine->EngineEvent(EngineStateEvent::ENGINE_LOAD_FINISHED);
  }

  void EngineLaunching::OnDetach() {
  }

  Ref<EngineState> EngineIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::CORRUPT_CONFIG_ERROR || event == EngineStateEvent::CORRUPT_SHADER_ERROR ||
        event == EngineStateEvent::ENGINE_FAILURE) {
      return NewRef<ErrorState>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      return NewRef<EngineShutdown>(engine);
    }

    if (event == EngineStateEvent::APP_ATTACHED) {
      OE_DEBUG("Loading application");
#ifdef OE_TESTING_ENVIRONMENT
      AppState::mode = EngineMode::TESTING;

      auto test_type = engine->config.GetVal<std::string>("TESTING", "TEST-TYPE");
      if (test_type.has_value() && *test_type != "ENGINE-TEST") {
        return NewRef<TestIdle>(engine);
      }
#endif  // OE_TESTING_ENVIRONMENT

      /// TODO: implement editor/runtime switch, option already processed, but automate launching with project manager
      bool in_editor = engine->cmd_line.HasFlag("--editor");
      Ref<EngineState> main_idle = nullptr;
      if (in_editor) {
        OE_DEBUG("Loading Editor");
        AppState::mode = EngineMode::EDITOR;
        main_idle = NewRef<EditorIdle>(engine);

        LoggerTargetData console_sink_data = {
          .sink = {
            .sink_name = "ConsoleSink",
            .sink_pattern = "%v",
            .level = spdlog::level::trace,
          },
          .sink_factory = []() -> spdlog::sink_ptr {
            return std::make_shared<EditorSink>();
          }
        };
        Logger::Instance()->RegisterTarget(console_sink_data);
      } else {
        OE_DEBUG("Loading Runtime");
        AppState::mode = EngineMode::RUNTIME;
        main_idle = NewRef<AppIdle>(engine);
      }

      AppState::AttachApplication();
      return main_idle;
    }  // namespace other

    return nullptr;
  }

  /// FIXME: dont go right to app attached
  void EngineIdle::OnAttach() {
    EventQueue::RegisterEventDispatcher<CreateFileEvent>(
      "Other-Engine--CreateFile",
      { &HandleCreateFile }
    );

    EventQueue::RegisterEventDispatcher<DeleteFileEvent>(
      "Other-Engine--DeleteFile",
      { &HandleDeleteFile }
    );
    /// TODO:
    // register event to listen for attached application
  }

  void EngineIdle::OnStep() {
    ScriptEngine::UpdateAttachments(engine->dt);
    engine->EngineEvent(EngineStateEvent::APP_ATTACHED);
  }

  void EngineIdle::OnDetach() {
    // EventQueue::UnregisterEventDispatcher("Other-Engine--AttachApplication");
  }

  Ref<EngineState> EngineShutdown::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::ENGINE_UNLOAD_FINISHED) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      engine->exit_code = AppState::exit_code.value();
      /// we never actually go back to engine idle, this is to avoid having a null state
    }

    return nullptr;
  }

  void EngineShutdown::OnAttach() {
    OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
    OE_DEBUG("Attaching Shutdown State");

    /// remove all event dispatchers
    /// TODO: remove user event dispatchers
    EventQueue::UnregisterEventDispatcher("Other-Engine--DeleteFile");
    EventQueue::UnregisterEventDispatcher("Other-Engine--CreateFile");
    EventQueue::UnregisterEventDispatcher("Other-Engine--Shutdown");
    EventQueue::UnregisterEventDispatcher("Other-Engine--WindowClosed");
  }

  void EngineShutdown::OnStep() {
    OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");

    /// clear event queue from any remaining events and flush one more event loop with no scene
    ///   to ensure we are in a stable state before shutting down
    EventQueue::Poll();

    ScriptEngine::Shutdown();
    UI::Shutdown();
    Renderer::Shutdown();
    AppState::Shutdown();

    OE_DEBUG("Engine Shutdown complete");
    engine->EngineEvent(EngineStateEvent::ENGINE_UNLOAD_FINISHED);
  }

  void EngineShutdown::OnDetach() {
  }

}  // namespace other
