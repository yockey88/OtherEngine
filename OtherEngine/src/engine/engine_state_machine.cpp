/**
 * \file engine/engine_state_machine.cpp
 **/
#include "engine/engine_state_machine.hpp"

#include "core/defines.hpp"
#include "core/filesystem.hpp"

#include "application/app_state.hpp"
#include "event/core_events.hpp"
#include "event/event_queue.hpp"
#include "event/key_events.hpp"
#include "event/window_events.hpp"

#include "scene/scene_manager.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_engine.hpp"

#include "engine/engine.hpp"

namespace other {
  namespace {

    bool HandleWindowClosed(WindowClosed& event) {
      EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
      return false;
    }

    bool HandleKeyPress(KeyPressed& event) {
      HandleKeyEvent(event, Keyboard::Key::OE_ESCAPE, [&]() {
        EventQueue::PushEvent<ShutdownEvent>({ ExitCode::SUCCESS });
      });

      return false;
    }

  }  // anonymous namespace

  Ref<EngineState> EngineLaunching::HandleEvent(const EngineStateEvent event) {
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
    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      return NewRef<EngineShutdown>(engine);
    }

    if (event == EngineStateEvent::APP_ATTACHED) {
      OE_DEBUG("Loading application");
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
    EventQueue::RegisterEventDispatcher<KeyPressed>(
      "Other-Engine--AttachApplication",
      {
        [&](KeyPressed& event) {
          HandleKeyEvent(event, Keyboard::Key::OE_F1, [&]() {
            engine->EngineEvent(EngineStateEvent::APP_ATTACHED);
          });
          return false;
        },
      }
    );
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

  Ref<EngineState> AppIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::APP_DETACHED) {
      OE_ASSERT(AppState::IsAttached(), "Application is not attached");
      AppState::DetachApplication();

      if (AppState::exit_code.has_value()) {
        return NewRef<EngineShutdown>(engine);
      } else {
        return NewRef<EngineIdle>(engine);
      }
    }

    if (event == EngineStateEvent::SCENE_LOADED) {
      return NewRef<SceneIdle>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      OE_ASSERT(AppState::exit_code.has_value(), "No exit code set for engine shutdown");
      return NewRef<EngineShutdown>(engine);
    }

    return nullptr;
  }

  void AppIdle::OnAttach() {
    if (!AppState::IsAttached()) {
      AppState::AttachApplication();
    }

    /// if primary-scene then load it, else either issue error (if runtime) or
    ///   open sandbox (if editor)
    auto& proj_data = AppState::ProjectContext()->GetMetadata();
    immediate_scene_load = proj_data.primary_scene.has_value();
  }

  void AppIdle::OnStep() {
    auto& proj_meta = AppState::ProjectContext()->GetMetadata();
    if (immediate_scene_load) {
      OE_ASSERT(proj_meta.primary_scene.has_value(), "No primary scene provided in project file");

      Ref<Directory> scene_dir = Filesystem::GetDirectory("scenes");
      OE_ASSERT(scene_dir != nullptr, "Failed to get scene directory");
      OE_ASSERT(scene_dir->Exists(), "Scene directory does not exist");

      Ref<FileHandle> primary_scene = scene_dir->GetFileHandleByName(*proj_meta.primary_scene);
      OE_ASSERT(primary_scene != nullptr, "Failed to get primary scene file handle");

      if (!AppState::Scenes()->LoadScene(*primary_scene)) {
        OE_ERROR("Failed to load primary scene : {}", *proj_meta.primary_scene);
        return;
      }

      AppState::Scenes()->SetAsActive(*primary_scene);
      engine->EngineEvent(EngineStateEvent::SCENE_LOADED);
      OE_DEBUG("Primary Scene Loaded : {}", *proj_meta.primary_scene);

      immediate_scene_load = false;
      return;
    }

    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

  void AppIdle::OnDetach() {
  }

  Ref<EngineState> SceneIdle::HandleEvent(const EngineStateEvent event) {
    if (event == EngineStateEvent::SCENE_UNLOADED) {
      return NewRef<AppIdle>(engine);
    }

    if (event == EngineStateEvent::ENGINE_SHUTDOWN) {
      /// this returns app idle so the scene gets unloaded and we queue
      ///   app detached event so that app-idle state gets immediately detached
      ///   and the engine can shutdown
      engine->EngineEvent(EngineStateEvent::APP_DETACHED);
      return NewRef<AppIdle>(engine);
    }

    return nullptr;
  }

  void SceneIdle::OnAttach() {
    OE_ASSERT(AppState::Scenes()->HasActiveScene(), "No active scene to attach");
    OE_ASSERT(AppState::Scenes()->ActiveScene() != nullptr, "Cannot attach null scene!");
    OE_ASSERT(AppState::Scenes()->ActiveScene()->scene != nullptr, "Active scene has no scene object");
    AppState::Scenes()->StartScene();
    OE_DEBUG("Scene Idle Attached : {}", AppState::Scenes()->ActiveScene()->name);
  }

  void SceneIdle::OnStep() {
    AppState::RunEarlyUpdate();
    AppState::RunUpdate();
    AppState::RunLateUpdate();
    AppState::HandleRender();
  }

  void SceneIdle::OnDetach() {
    AppState::Scenes()->UnloadActive();
  }

}  // namespace other