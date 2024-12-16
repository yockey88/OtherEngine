/**
 * \file sandbox/main.cpp
 **/
#include "asset/asset_manager.hpp"

#include "rendering/model.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/scene_renderer.hpp"

#include "other_engine.hpp"

using namespace other;

class SandboxApp : public App {
 public:
  SandboxApp(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~SandboxApp() override {}

  AssetHandle debug_quad;

  virtual void OnAttach() override {
    OE_INFO("Sandbox App Attached");

    debug_quad = ModelFactory::CreateFramebufferMesh();
  }

  virtual void Render() override {
  }

  virtual Ref<SceneRenderer> CreateSceneRenderer() override {
    SceneRenderSpec spec = Renderer::GetDefaultSceneSpec();
    PipelineSpec shadow_map_pipeline = {
      .framebuffer_spec = {
        .depth_func = LESS,
        .clear_color = { 0.1f, 0.1f, 0.1f, 0.5f },
        .size = { 1920, 1080 },
      },
      .pipeline_name = "ShadowMap",
    };
    PipelineSpec depth_pipeline = {
      .framebuffer_spec = {
        .depth_func = LESS,
        .clear_color = { 0.f, 0.f, 0.f, 1.f },
        .size = { 1920, 1080 },
      },
      .pipeline_name = "Depth",
    };
    spec.pipelines.push_back(shadow_map_pipeline);
    spec.pipelines.push_back(depth_pipeline);
    spec.pipeline_passes[FNV("ShadowMap")].passes.push_back(SceneRenderer::SHADOW_MAP);
    spec.pipeline_passes[FNV("Depth")].passes.push_back(SceneRenderer::DEPTH_PASS);

    return NewRef<SceneRenderer>(spec);
  }
};

OTHER_ENTRY_POINT(SandboxApp);