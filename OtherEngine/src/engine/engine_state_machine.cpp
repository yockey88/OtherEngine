/**
 * \file engine/engine_state_machine.cpp
 **/
#include "engine/engine_state_machine.hpp"

#include "core/config_keys.hpp"

#include "application/app_state.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

#include "editor/editor.hpp"
#include "engine/engine.hpp"

namespace other {

  Ref<EngineState> EngineLaunching::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::ENGINE_LOAD_FINISHED) {
      return NewRef<EngineIdle>(engine);
    }

    return nullptr;
  }

  void EngineLaunching::OnAttach() {
    EventQueue::RegisterEventDispatcher<WindowClosed>(
      "Other-Engine--WindowClosed",
      {
        [&](WindowClosed& event) {
          EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
          return false;
        },
      }
    );

    EventQueue::RegisterEventDispatcher<ShutdownEvent>(
      "Other-Engine--Shutdown",
      {
        [&](ShutdownEvent& event) {
          engine->exit_code = event.exit_code;
          engine->EngineEvent(EngineStateEvent::ENGINE_SHUTDOWN);
          return false;
        },
      }
    );
  }

  void EngineLaunching::OnStep() {
    /// TODO: implement actual engine launch logic,
    ///       - check if headless mode
    ///       - check configuration settings (server, client, editor, runtime, etc....)
    ///       - push engine load complete event
    engine->EngineEvent(EngineStateEvent::ENGINE_LOAD_FINISHED);
  }

  Ref<EngineState> EngineIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::APP_ATTACHED) {
      OE_DEBUG("Loading application");
      AppState::AttachApplication();

      bool in_editor = engine->cmd_line.HasFlag("--editor");
      if (in_editor) {
        // Ref<Layer> core_layer = nullptr;
        // bool debug_editor = config.GetVal<bool>(kDebugSection, "EDITOR").value_or(false);
        // if (debug_editor) {
        //   core_layer = NewRef<TEditorLayer>(&active_app, active_app.config);
        // } else {
        //   core_layer = NewRef<EditorLayer>(&active_app, active_app.config);
        // }

        // AppState::PushLayer(core_layer);

        // Logger::Instance()->RegisterTarget({
        //   .target_name = "Editor-Console",
        //   .level = Logger::LevelFromLevel(Logger::Level::DEBUG),
        //   .log_format = "%v",
        //   .sink_factory = []() -> spdlog::sink_ptr {
        //     return NewStdRef<EditorConsoleSink>(10);
        //   },
        // });

        AppState::mode = EngineMode::EDITOR;
      } else {
        AppState::mode = EngineMode::RUNTIME;
        /// push runtime layers
      }
      return NewRef<AppIdle>(engine);
    }

    return nullptr;
  }

  void EngineIdle::OnAttach() {
    AppState::Initialize(engine->cmd_line, engine->config);
    Renderer::Initialize(engine->config);
    UI::Initialize(engine->config, Renderer::GetWindow());
    ScriptEngine::Initialize(engine->config);
    PhysicsEngine::Initialize(engine->config);

    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "Other-Engine-Core-KeyPressed",
      {
        [&](KeyPressed& event) {
          HandleKeyEvent(event, Keyboard::Key::OE_ESCAPE, [&]() {
            EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
          });

          HandleKeyEvent(event, Keyboard::Key::OE_F1, [&]() {
            engine->EngineEvent(EngineStateEvent::APP_ATTACHED);
          });

          return false;
        },
      }
    );
  }

  void EngineIdle::OnStep() {
    /// TODO: check if we are shutting down here and perform shutdown operations
  }

  void EngineIdle::OnDetach() {
    EventQueue::UnregisterEventDispatcher("Other-Engine-Core-KeyPressed");
  }

  Ref<EngineState> EngineShutdown::HandleEvent(const EngineStateEvent event) {
    return nullptr;
  }

  void EngineShutdown::OnStep() {
    PhysicsEngine::Shutdown();
    ScriptEngine::Shutdown();
    UI::Shutdown();
    Renderer::Shutdown();
    AppState::Shutdown();

    engine->EngineEvent(EngineStateEvent::ENGINE_UNLOAD_FINISHED);
  }

  Ref<EngineState> AppIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::APP_DETACHED) {
      AppState::DetachApplication();
      return NewRef<EngineIdle>(engine);
    }

    return nullptr;
  }

  void AppIdle::OnStep() {}

  Ref<EngineState> SceneIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::SCENE_UNLOADED) {
      return NewRef<AppIdle>(engine);
    }

    return nullptr;
  }

  void SceneIdle::OnStep() {
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

}  // namespace other