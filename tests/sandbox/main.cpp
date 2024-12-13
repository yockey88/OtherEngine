/**
 * \file sandbox/main.cpp
 **/
#include "other_engine.hpp"

using namespace other;

class SandboxApp : public App {
 public:
  SandboxApp(const other::CmdLine& cmd_line, const other::ConfigTable& config)
      : other::App(cmd_line, config) {}
  virtual ~SandboxApp() override {}

  virtual void OnAttach() override {
    OE_INFO("Sandbox App Attached");
  }

  virtual Ref<SceneRenderer> CreateSceneRenderer() override {
    SceneRenderSpec spec = Renderer::GetDefaultSceneSpec();

    Ref<Directory> shader_dir = Filesystem::GetDirectory("shaders");
    Ref<FileHandle> shader = shader_dir->GetFile("depth_shader.oshader");
    if (shader == nullptr) {
      OE_ERROR("Failed to get shader file : {}", "depth_shader.oshader");
      return Renderer::DefaultSceneRenderer();
    }

    Ref<Shader> depth_shader = BuildShader(shader->AbsolutePath());

    RenderPassSpec depth_pass = {
      .name = "depth-pass",
      .tag_col = { 0.f, 0.f, 0.f, 1.f },
      .shader = depth_shader,
    };
    PipelineSpec depth_pipeline = {
      .topology = DrawMode::TRIANGLES,
      .framebuffer_spec = {
        .depth_func = LESS,
        .clear_color = { 0.f, 0.f, 0.f, 1.f },
        .size = { 1920, 1080 },
      },
      .pipeline_name = "Depth",
    };

    spec.passes.push_back(NewRef<RenderPass>(depth_pass));
    spec.pipelines.push_back(depth_pipeline);
    spec.pipeline_to_pass_map[FNV("Depth")].push_back(FNV("depth-pass"));

    return NewRef<SceneRenderer>(spec);
  }
};

OTHER_ENTRY_POINT(SandboxApp);