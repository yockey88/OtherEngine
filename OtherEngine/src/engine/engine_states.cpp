/**
 * \file engine/engine_states.cpp
 **/
#include "engine/engine_states.hpp"

#include "engine/editor_states.hpp"
#include "engine/engine.hpp"

#include "application/app_state.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

namespace other {
  namespace {

    bool HandleWindowClosed(WindowClosed& event) {
      EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
      return false;
    }

    bool HandleKeyPress(KeyPressed& event) {
      /// TODO: remove this, just for fast development iteration
      HandleKeyEvent(event, Keyboard::Key::OE_ESCAPE, [&]() {
        EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
      });

      return false;
    }

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
    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "Other-Engine--KeyPressed",
      { &HandleKeyPress }
    );
  }

  void EngineLaunching::OnStep() {
    /// TODO: implement actual engine launch logic,
    ///       - check if headless mode
    ///       - check configuration settings (server, client, editor, runtime, etc....)
    if (engine->cmd_line.HasFlag("--headless")) {
      AppState::mode = EngineMode::HEADLESS;
    }

    AppState::Initialize(engine->cmd_line, engine->config);
    Renderer::Initialize(engine->config);
    UI::Initialize(engine->config, Renderer::GetWindow());
    ScriptEngine::Initialize(engine->config);
    PhysicsEngine::Initialize(engine->config);

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

      /// TODO: implement editor/runtime switch, option already processed, but automate launching with project manager
      bool in_editor = engine->cmd_line.HasFlag("--editor");
      Ref<EngineState> main_idle = nullptr;
      if (in_editor) {
        OE_DEBUG("Loading Editor");
        AppState::mode = EngineMode::EDITOR;
        main_idle = NewRef<EditorIdle>(engine);
      } else {
        OE_DEBUG("Loading Runtime");
        AppState::mode = EngineMode::RUNTIME;
        main_idle = NewRef<AppIdle>(engine);
      }

#ifdef OE_TESTING_ENVIRONMENT
      AppState::mode = EngineMode::TESTING;
#endif  // OE_TESTING_ENVIRONMENT

      return main_idle;
    }

    return nullptr;
  }

  void EngineIdle::OnAttach() {
    /// TODO: do we go straight from engine launching to app idle???
    engine->EngineEvent(EngineStateEvent::APP_ATTACHED);
    // EventQueue::RegisterEventDispatcher<KeyPressed>(
    //   "Other-Engine--AttachApplication",
    //   {
    //     [&](KeyPressed& event) {
    //       HandleKeyEvent(event, Keyboard::Key::OE_F1, [&]() {
    //         engine->EngineEvent(EngineStateEvent::APP_ATTACHED);
    //       });
    //       return false;
    //     },
    //   }
    // );
  }

  void EngineIdle::OnStep() {
    ScriptEngine::UpdateAttachments(engine->dt);
  }

  void EngineIdle::OnDetach() {
    EventQueue::UnregisterEventDispatcher("Other-Engine--AttachApplication");
  }

  Ref<EngineState> EngineShutdown::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::ENGINE_UNLOAD_FINISHED) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      engine->exit_code = AppState::exit_code.value();
      /// we never actually go back to engine idle, this is to avoid having a null state

      OE_INFO("Engine Exit : {}", engine->exit_code.value());
    }

    return nullptr;
  }

  void EngineShutdown::OnAttach() {
    OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
    OE_DEBUG("Attaching Shutdown State");

    /// remove all event dispatchers
    /// TODO: remove user event dispatchers
    EventQueue::UnregisterEventDispatcher("Other-Engine--KeyPressed");
    EventQueue::UnregisterEventDispatcher("Other-Engine--Shutdown");
    EventQueue::UnregisterEventDispatcher("Other-Engine--WindowClosed");
  }

  void EngineShutdown::OnStep() {
    OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");

    /// clear event queue from any remaining events and flush one more event loop with no scene
    ///   to ensure we are in a stable state before shutting down
    EventQueue::Poll();

    PhysicsEngine::Shutdown();
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
