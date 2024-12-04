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

#include "editor/editor_asset_handler.hpp"

namespace other {

  App::App(const CmdLine& cmdline, const ConfigTable& config)
      : cmdline(cmdline), config(config) {}

  App::~App() {}

  void App::Load() {
    OnLoad();
  }

  void App::Unload() {
    OnUnload();
  }

  Ref<AssetHandler> App::CreateAssetHandler(EngineMode mode) {
    if (mode == EngineMode::EDITOR) {
      return NewRef<EditorAssetHandler>();
    } else {
      return NewRef<RuntimeAssetHandler>();
    }
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

  void App::OnRender() {
    Render();
  }

  void App::OnRenderUI() {
    RenderUI();
  }

  void App::Detach() {
    OnDetach();
  }

}  // namespace other
