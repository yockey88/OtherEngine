/**
 * \file application\app.cpp
 */
#include "application\app.hpp"

#include <string_view>

#include "core/logger.hpp"

#include "application/app_state.hpp"
#include "asset/asset_database.hpp"
#include "event/event_queue.hpp"
#include "parsing/cmd_line_parser.hpp"

#include "rendering/framebuffer.hpp"
#include "rendering/render_pass.hpp"
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
      OE_CRITICAL("NO RUNTIME ASSET HANDLER IMPLEMENTED");
      return NewRef<EditorAssetHandler>();

      // OE_ASSERT(false, "NO RUNTIME ASSET HANDLER IMPLEMENTED");
      // return NewRef<RuntimeAssetHandler>();
    }
  }

  Ref<SceneRenderer> App::CreateSceneRenderer() {
    auto framebuffer_specs = config.GetFramebufferSpecs();
    auto render_passes = config.GetRenderPasses();
    auto pipelines = config.GetPipelines();

    std::stringstream ss;
    for (const auto& [_, spec] : framebuffer_specs) {
      ss = std::stringstream(spec);
      FramebufferSpec fb = Reader<FramebufferSpec>{}(ss);
      OE_DEBUG("Loaded framebuffer spec: {}", fb.framebuffer_name);
      render_specs.framebuffer_specs.push_back(fb);
    }

    std::vector<RenderPassSpec> passes;
    for (const auto& [_, pass] : render_passes) {
      ss = std::stringstream(pass);
      render_specs.render_passes.push_back(Reader<RenderPassSpec>{}(ss));
    }

    std::vector<PipelineSpec> pipes;
    for (const auto& [_, pipe] : pipelines) {
      ss = std::stringstream(pipe);
      render_specs.pipelines.push_back(Reader<PipelineSpec>{}(ss));
    }

    return Renderer::ConstructSceneRenderer(&render_specs);
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
