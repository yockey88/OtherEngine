/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include <string_view>

#include "core/logger.hpp"
#include "core/time.hpp"

#include "application/app_state.hpp"
#include "asset/runtime_asset_handler.hpp"
#include "event/event_queue.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "rendering/renderer.hpp"
#include "scripting/script_defines.hpp"

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
    // do {
    //   time::DeltaTime::Update();
    //   float dt = time::DeltaTime::Get();

    //   DoEarlyUpdate(dt);
    //   DoUpdate(dt);
    //   DoLateUpdate(dt);

    //   DoRender();
    //   DoRenderUI();

    //   EventQueue::Poll();
    // } while (!exit_code.has_value());

    Detach();

    OE_DEBUG("Application successfully detached");
  }

  void App::Unload() {
    OnUnload();
  }

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

  Ref<SceneRenderer> App::CreateSceneRenderer() {
    return Renderer::DefaultSceneRenderer();
  }

  void App::Attach() {
    OnAttach();
  }

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
  }

  void App::DoRenderUI() {
    RenderUI();
  }

  void App::Detach() {
    OnDetach();
  }

}  // namespace other
