/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include <filesystem>
#include <string_view>

#include "core/config_keys.hpp"
#include "core/engine.hpp"
#include "core/engine_state.hpp"
#include "core/logger.hpp"
#include "core/time.hpp"

#include "application/app_state.hpp"
#include "asset/runtime_asset_handler.hpp"
#include "event/app_events.hpp"
#include "event/core_events.hpp"
#include "event/event_handler.hpp"
#include "event/event_queue.hpp"
#include "event/ui_events.hpp"
#include "input/io.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/ui/ui.hpp"
#include "scripting/script_defines.hpp"
#include "scripting/script_engine.hpp"

namespace other {

  App::App(const CmdLine& cmdline, const ConfigTable& config)
      : cmdline(cmdline), config(config) {}

  App::~App() {}

  void App::Load() {
    OnLoad();
  }

  void App::Run() {
    Attach();

    time::DeltaTime delta_time;
    delta_time.Start();
    while (!EngineState::exit_code.has_value()) {
      float dt = delta_time.Get();

      EventQueue::Poll();
      if (Renderer::IsWindowFocused()) {
        DoEarlyUpdate(dt);
        DoUpdate(dt);
      }

      Renderer::GetWindow()->Clear();
      DoRender();
      DoRenderUI();
      Renderer::GetWindow()->SwapBuffers();

      CHECKGL();
    }

    Detach();

    OE_DEBUG("Application successfully detached");
  }

  void App::Unload() {
    OnUnload();
  }

  // void App::LoadScene(const Path& path) {
  //   scene_manager->SetAsActive(path);
  //   const SceneMetadata* scn_metadata = ActiveScene();
  //   if (scn_metadata == nullptr) {
  //     OE_ERROR("Failed to load scene : {}", path);
  //     return;
  //   }
  //   /// alert the client app new scene is loaded
  //   OnSceneLoad(ActiveScene());
  //   /// propogate scene loading through layers
  //   for (auto& l : *layer_stack) {
  //     l->LoadScene(scn_metadata);
  //   }
  // }

  // void App::UnloadScene() {
  //   if (ActiveScene() == nullptr) {
  //     return;
  //   }
  //   /// Do we need to save before we offload
  //   // scene_manager->SaveActiveScene();
  //   /// alert client app about scene unload
  //   OnSceneUnload();
  //   OE_DEBUG("Unloading scene : {}", ActiveScene()->path);
  //   scene_manager->UnloadActive();
  //   /// propogate scene loading through layers
  //   for (auto& l : *layer_stack) {
  //     l->UnloadScene();
  //   }
  // }

  // void App::ReloadScripts() {
  //   bool scene_playing = false;
  //   Opt<Path> active_path = std::nullopt;
  //   if (scene_manager->ActiveScene() != nullptr) {
  //     scene_playing = scene_manager->IsPlaying();
  //     active_path = scene_manager->ActiveScene()->path;
  //     UnloadScene();
  //   }

  //   scene_manager->ClearScenes();
  //   ScriptEngine::ReloadAllScripts();

  //   OnScriptReload();

  //   if (active_path.has_value()) {
  //     LoadScene(active_path.value());
  //   }

  //   if (scene_playing) {
  //     scene_manager->StartScene();
  //   }

  //   for (auto& l : *layer_stack) {
  //     l->ReloadScripts();
  //   }
  // }

  Ref<AssetHandler> App::CreateAssetHandler() {
    return NewRef<RuntimeAssetHandler>();
  }

  void App::Attach() {
    CHECKGL();

    EventQueue::RegisterEventDispatcher<ShutdownEvent>(
      "Shutdown-Event",
      {
        [](ShutdownEvent& e) -> bool {
          EngineState::exit_code = e.exit_code;
          return true;
        },
      }
    );
    EventQueue::RegisterEventDispatcher<UIWindowClosed>(
      "Remove-UI-Window",
      {
        [this](UIWindowClosed& e) -> bool {
          // return RemoveUIWindow(e.GetWindowId());
          return true;
        },
      }
    );
    EventQueue::RegisterEventDispatcher<ScriptReloadEvent>(
      "Reload-Scripts",
      {
        [this](ScriptReloadEvent& e) -> bool {
          // ReloadScripts();
          return true;
        },
      }
    );

    OnAttach();
  }

  /// TODO: add early update to layers and scene
  void App::DoEarlyUpdate(float dt) {
    EarlyUpdate(dt);
  }

  void App::DoUpdate(float dt) {
    Update(dt);
  }

  void App::DoLateUpdate(float dt) {
    LateUpdate(dt);
  }

  void App::DoRender() {
    Render();
    CHECKGL();
  }

  void App::DoRenderUI() {
    RenderUI();
  }

  void App::Detach() {
    OnDetach();
  }

}  // namespace other
