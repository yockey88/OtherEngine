/**
 * \file rendering\renderer.cpp
 */
#include "rendering\renderer.hpp"

#include <glad/glad.h>
#include <imgui/imgui.h>

#include "core/filesystem.hpp"

#include "application/app_state.hpp"

#include "rendering/geometry_pass.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/scene_renderer.hpp"
#include "rendering/shader.hpp"

namespace other {

  Scope<Window> Renderer::window = nullptr;
  Ref<Scene> Renderer::scene_ctx = nullptr;
  Ref<VertexArray> Renderer::window_mesh = nullptr;
  Ref<Shader> Renderer::window_shader = nullptr;

  void Renderer::Initialize(const ConfigTable& config) {
    auto win_cfg = Window::ConfigureWindow(config);
    auto win_res = Window::GetWindow(win_cfg, config);

    CHECKGL();

    if (win_res.IsErr()) {
      throw std::runtime_error("Failed to create engine window!");
    }

    window = std::move(win_res.Unwrap());

    /// TODO: configure window shader and mesh using config
    const Path win_shader_path = Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders" / "fbshader.oshader";
    window_shader = BuildShader(win_shader_path);

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
    window_mesh = NewRef<VertexArray>(fb_verts, fb_indices, fb_layout);

    CHECKGL();
  }

  void Renderer::SetSceneContext(const Ref<Scene>& scene) {
    scene_ctx = scene;
  }

  glm::ivec2 Renderer::WindowSize() {
    return window->Size();
  }

  glm::ivec2 Renderer::WindowPos() {
    return window->Position();
  }

  void Renderer::HandleWindowResize(const glm::ivec2& size) {
    window->Resize(size);
  }

  bool Renderer::IsWindowFocused() {
    return window->HasFocus() || ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);
  }

  void Renderer::Shutdown() {
    scene_ctx = nullptr;
    window = nullptr;
  }

  const Scope<Window>& Renderer::GetWindow() {
    return window;
  }

  void Renderer::SetWindowClearColor(const std::vector<std::string>& color) {
    glm::vec4 c;
    for (int i = 0; i < color.size(); i++) {
      c[i] = std::stof(color[i]);
    }
    window->SetClearColor(c);
  }

  void Renderer::SetWindowClearColor(const glm::vec4& color) {
    window->SetClearColor(color);
  }

  void Renderer::DrawFramebufferToWindow(const Ref<Framebuffer>& framebuffer) {
    OE_ASSERT(framebuffer != nullptr, "Cannot render null framebuffer to window!");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, framebuffer->texture);
    window_shader->Bind();
    window_shader->SetUniform("oe_screen_tex", 0);
    window_shader->SetUniform("exposure", 1.f);
    window_mesh->Draw(TRIANGLES);
    window_shader->Unbind();
  }

  Ref<SceneRenderer> Renderer::DefaultSceneRenderer() {
    Layout default_layout = {
      { ValueType::VEC3, "position" },
      { ValueType::VEC3, "normal" },
      { ValueType::VEC3, "tangent" },
      { ValueType::VEC3, "binormal" },
      { ValueType::VEC2, "uvs" }
    };

    uint32_t model_binding_pnt = 1;
    std::vector<Uniform> model_unis = {
      { "models", ValueType::MAT4, 100 },
    };

    uint32_t material_binding_pnt = 2;
    std::vector<Uniform> material_unis = {
      { "materials", ValueType::USER_TYPE, 100, sizeof(Material) },
    };

    Path engine_core_dir = Filesystem::GetEngineCoreDir();
    Path assets_dir = engine_core_dir / "OtherEngine" / "assets";
    Path shader_dir = assets_dir / "shaders";
    Path default_path = shader_dir / "default.oshader";

    std::vector<Uniform> geometry_unis = {};
    Ref<Shader> geometry_shader = BuildShader(default_path);
    Ref<RenderPass> geom_pass = NewRef<GeometryPass>(geometry_unis, geometry_shader);

    SceneRenderSpec spec{
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
      },
      .passes = { geom_pass },
      .pipeline_to_pass_map = {
        {
          FNV("Geometry"),
          { FNV(geom_pass->Name()) },
        },
      },
    };
    return NewRef<SceneRenderer>(spec);
  }

}  // namespace other
