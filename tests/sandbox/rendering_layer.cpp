/**
 * \file rendering_layer.cpp
 **/
#include "rendering_layer.hpp"

#include "core/filesystem.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "input/mouse.hpp"

#include "rendering/geometry_pass.hpp"
#include "rendering/outline_pass.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/renderer.hpp"
#include "scripting/script_engine.hpp"


using namespace other;

void RenderingLayer::OnAttach() {
  EventQueue::RegisterEventDispatcher<KeyPressed>(
    "Rendering-Layer-Key-Bindings",
    {
      std::bind_front(&RenderingLayer::HandleKeyPressed, this),
    }
  );

  Path engine_core_dir = Filesystem::GetEngineCoreDir();
  Path assets_dir = engine_core_dir / "OtherEngine" / "assets";

  shader_dir = assets_dir / "shaders";
  default_path = shader_dir / "default.oshader";
  normals_path = shader_dir / "normals.oshader";
  fbshader_path = shader_dir / "fbshader.oshader";
  deferred_shader_path = shader_dir / "deferred_shading.oshader";
  add_fog_shader_path = shader_dir / "fog.oshader";
  red_path = shader_dir / "red.oshader";
  outline_path = shader_dir / "outline.oshader";
  pure_geometry_path = shader_dir / "pure_geometry.oshader";

  auto win_size = Renderer::WindowSize();

  camera = NewRef<PerspectiveCamera>(glm::ivec2{ win_size.x, win_size.y });
  camera->SetPosition({ 0.f, 0.f, 3.f });
  DefaultUpdateCamera(camera);

  Ref<Shader> fbshader = BuildShader(fbshader_path);

  std::vector<float> fb_verts = {
    1.f, 1.f, 1.f, 1.f,
    -1.f, 1.f, 0.f, 1.f,
    -1.f, -1.f, 0.f, 0.f,
    1.f, -1.f, 1.f, 0.f
  };

  std::vector<uint32_t> fb_indices = {
    0, 1, 3,
    1, 2, 3
  };
  std::vector<uint32_t> fb_layout = {
    2, 2
  };

  Ref<VertexArray> fb_mesh = NewRef<VertexArray>(fb_verts, fb_indices, fb_layout);
  Ref<Framebuffer> framebuffer = NewRef<Framebuffer>(FramebufferSpec{
    .size = win_size,
  });

  uint32_t model_binding_pnt = 1;
  std::vector<Uniform> model_unis = {
    { "models", ValueType::MAT4, 100 },
  };

  uint32_t material_binding_pnt = 2;
  std::vector<Uniform> material_unis = {
    { "materials", ValueType::USER_TYPE, 100, sizeof(Material) },
  };

  Layout default_layout = {
    { ValueType::VEC3, "position" },
    { ValueType::VEC3, "normal" },
    { ValueType::VEC3, "tangent" },
    { ValueType::VEC3, "binormal" },
    { ValueType::VEC2, "uvs" }
  };

  std::vector<Uniform> outline_unis = {
    { "outline_color", VEC3 },
  };
  std::vector<Uniform> geometry_unis = {};
  Ref<Shader> outline_shader = BuildShader(outline_path);
  Ref<Shader> geometry_shader = BuildShader(default_path);

  RenderPassSpec pure_geom_pass_spec{
    .name = "Geometry",
    .tag_col = { 0.f, 0.f, 1.f, 1.f },
    .uniforms = {},
    .shader = BuildShader(pure_geometry_path),
  };
  RenderPassSpec normal_pass_spec{
    .name = "Normals",
    .tag_col = { 0.f, 1.f, 0.f, 1.f },
    .uniforms = {
      { "magnitude", ValueType::FLOAT },
    },
    .shader = BuildShader(normals_path),
  };
  pure_geom_pass = NewRef<RenderPass>(pure_geom_pass_spec);
  geom_pass = NewRef<GeometryPass>(geometry_unis, geometry_shader);
  outline_pass = NewRef<OutlinePass>(outline_unis, outline_shader);
  normal_pass = NewRef<RenderPass>(normal_pass_spec);
  normal_pass->SetInput("magnitude", 0.2f);

  SceneRenderSpec render_spec{
    .pipelines = {
      {
        .topology = DrawMode::TRIANGLES,
        .framebuffer_spec = {
          .depth_func = LESS_EQUAL,
          .clear_color = { 0.1f, 0.1f, 0.1f, 1.f },
          .size = Renderer::WindowSize(),
        },
        .vertex_layout = default_layout,
        .model_uniforms = model_unis,
        .model_binding_point = model_binding_pnt,
        .material_uniforms = material_unis,
        .material_binding_point = material_binding_pnt,
        .debug_name = "Geometry",
      },
      {
        .topology = DrawMode::TRIANGLES,
        .framebuffer_spec = {
          .depth_func = LESS_EQUAL,
          .clear_color = { 0.1f, 0.1f, 0.1f, 1.f },
          .size = Renderer::WindowSize(),
        },
        .vertex_layout = default_layout,
        .model_uniforms = model_unis,
        .model_binding_point = model_binding_pnt,
        .material_uniforms = material_unis,
        .material_binding_point = material_binding_pnt,
        .debug_name = "Debug",
      },
    },
    .passes = { geom_pass, normal_pass, pure_geom_pass },
    .pipeline_to_pass_map = {
      {
        FNV("Geometry"),
        { FNV(geom_pass->Name()) },
      },
      {
        FNV("Debug"),
        { FNV(pure_geom_pass->Name()) },
      },
    },
  };

  renderer = NewRef<SceneRenderer>(render_spec);
  AppState::Scenes()->LoadRenderer(renderer);

  sandbox_ui = ScriptEngine::GetScriptObject("SandboxUI", "", "sandbox_ui");
  OE_ASSERT(sandbox_ui != nullptr, "Failed to load sandbox ui script object");

  sandbox_ui->Initialize();
}

void RenderingLayer::OnDetach() {
  sandbox_ui->Shutdown();
  sandbox_ui = nullptr;
}

void RenderingLayer::OnLateUpdate(float dt) {
  if (camera == nullptr) {
    return;
  }

  if (!camera_lock) {
    DefaultUpdateCamera(camera);
  }

  sandbox_ui->Update(dt);

  renderer->SubmitCamera(camera);
}

void RenderingLayer::OnRender() {
  sandbox_ui->Render();
}

void RenderingLayer::OnUIRender() {
  sandbox_ui->RenderUI();
}

bool RenderingLayer::HandleKeyPressed(KeyPressed& event) {
  HandleKeyEvent(event, Keyboard::Key::OE_C, [&](KeyPressed& event) {
    camera_lock = !camera_lock;
    if (camera_lock) {
      Mouse::FreeCursor();
    } else {
      Mouse::LockCursor();
    }
  });

  return event.Key() != Keyboard::Key::OE_C;
}