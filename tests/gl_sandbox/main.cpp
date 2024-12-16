/**
 * \file gl_sandbox/main.cpp
 **/
#include <iostream>

#include <SDL_events.h>
#include <SDL_video.h>
#include <box2d/b2_world.h>
#include <box2d/box2d.h>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/imgui.h>
#include <rendering/gbuffer.hpp>

#include "core/defines.hpp"
#include "core/errors.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/ref.hpp"
#include "engine/engine.hpp"

#include "application/app_state.hpp"
#include "event/event_queue.hpp"
#include "input/io.hpp"

#include "physics/phyics_engine.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/direction_light.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/material.hpp"
#include "rendering/material_table.hpp"
#include "rendering/perspective_camera.hpp"
#include "rendering/point_light.hpp"
#include "rendering/renderer.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/shader.hpp"
#include "rendering/ui/ui.hpp"
#include "rendering/uniform.hpp"
#include "rendering/vertex.hpp"
#include "rendering/window.hpp"
#include "scripting/script_engine.hpp"

#include "gl_helpers.hpp"
#include "sandbox_ui.hpp"
#include "shader_embed.hpp"

struct Quad {
  uint32_t vao = 0, vbo = 0, ebo = 0;
  std::vector<float> vertices;
  constexpr static uint32_t indices[] = {
    0,
    1,
    2,
    1,
    2,
    3,
  };

  Quad();
  ~Quad();

  void Draw();
};

struct Cube {
  uint32_t vao = 0, vbo = 0, ebo = 0;
  std::vector<float> vertices;
  constexpr static uint32_t indices[] = {
    0,
    1,
    2,
    2,
    3,
    0,
    1,
    5,
    6,
    6,
    2,
    1,
    7,
    6,
    5,
    5,
    4,
    7,
    4,
    0,
    3,
    3,
    7,
    4,
    4,
    5,
    1,
    1,
    0,
    4,
    3,
    2,
    6,
    6,
    7,
    3,
  };

  Cube();
  ~Cube();

  void Draw(other::DrawMode mode, uint32_t instances = 1);
};

constexpr static uint32_t win_w = 800;
constexpr static uint32_t win_h = 600;

using namespace other;

void UpdateCamera(other::Ref<CameraBase>& camera);

uint32_t BuildMaterialTable(uint32_t mip_levels, const glm::vec2& size, const std::vector<glm::vec3>& colors);

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  other::CmdLine cmd_line(__argc, __argv);
#else
int main(int argc, char* argv[]) {
  other::CmdLine cmd_line(argc, argv);
#endif
  int exit = 0;
  try {
    const other::Path glsandbox_dir = "C:/Yock/code/OtherEngine/tests/gl_sandbox";
    const other::Path config_path = glsandbox_dir / "gl_sandbox.other";

    cmd_line.SetFlag("--project", { config_path.string() });

    other::Engine mock_engine(cmd_line, "Sandbox--Thread");

    AppState::Initialize(&mock_engine);
    Renderer::Initialize(mock_engine.config);
    UI::Initialize(mock_engine.config, Renderer::GetWindow());
    ScriptEngine::Initialize(mock_engine.config);
    PhysicsEngine::Initialize(mock_engine.config);
    AppState::AttachApplication();
    AppState::mode = EngineMode::EDITOR;

    OE_DEBUG("GL Sandbox Launched");

    uint32_t shader1 = other::GetShader(vert1, frag1);
    uint32_t shader2 = other::GetShader(vert2, frag2);

    OE_INFO("Shaders Compiled");

    Quad quad;
    CHECKGL();
    Cube cube;
    CHECKGL();

    OE_INFO("VAOs created");

    Ref<Framebuffer> frame = NewRef<Framebuffer>(other::FramebufferSpec{});

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

    Scope<VertexArray> fb_mesh = NewScope<VertexArray>(fb_verts, fb_indices, fb_layout);

    const other::Path shader_dir = other::Filesystem::GetEngineCoreDir() / "OtherEngine" / "assets" / "shaders";
    const other::Path fbshader_path = shader_dir / "fbshader.oshader";
    const other::Path gbuffer_shader_path = shader_dir / "gbuffer.oshader";
    Ref<Shader> fb_shader = other::BuildShader(fbshader_path);
    Ref<Shader> gbuffer_shader = other::BuildShader(gbuffer_shader_path);

    // const other::Path deferred_shader_path = shader_dir / "deferred_shading.oshader";
    // Ref<Shader> deferred_shader = other::BuildShader(deferred_shader_path);
    // deferred_shader->Bind();
    // deferred_shader->SetUniform("goe_position", 0);
    // deferred_shader->SetUniform("goe_normal", 1);
    // deferred_shader->SetUniform("goe_albedo", 2);
    // deferred_shader->Unbind();

    const other::Path gl_sb_material_dir = other::Filesystem::GetEngineCoreDir() / "tests" / "gl_sandbox" / "shaders";
    const other::Path mat_path = gl_sb_material_dir / "mat.oshader";

    Ref<Shader> mat_shader = other::BuildShader(mat_path);
    // Ref<Shader> mat2_shader = other::BuildShader(mat2_path);

    other::Ref<CameraBase> camera = other::NewRef<PerspectiveCamera>(glm::ivec2{ win_w, win_h });
    camera->SetPosition({ 0.f, 0.f, 3.f });
    glm::mat4 proj = camera->ProjectionMatrix();
    glm::mat4 view = camera->ViewMatrix();

    glm::mat4 model1 = glm::mat4(1.0f);
    float m1_rotation = 0.f;

    glm::mat4 model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(2.f, 0.f, 0.f));

    other::PointLight point_light{
      .position = { 1.2f, 1.0f, 2.0f, 1.f },
      .color = { 0.2f, 0.2f, 0.2f, 1.f },
    };
    other::DirectionLight dir_light{
      .direction = { -0.2f, 1.0f, -0.3f, 1.f },
      .color = { 1.0f, 1.0f, 1.0f, 1.f },
    };

    CHECKGL();

    uint32_t camera_binding_pnt = 0;
    std::vector<other::Uniform> camera_unis = {
      { "projection", other::ValueType::MAT4 },
      { "view", other::ValueType::MAT4 },
      { "viewpoint", other::ValueType::VEC4 },
    };

    uint32_t model_binding_pnt = 1;
    std::vector<other::Uniform> model_unis = {
      { "models", other::ValueType::MAT4, 100 },
    };

    uint32_t material_binding_pnt = 2;
    std::vector<other::Uniform> material_unis = {
      { "materials", other::ValueType::USER_TYPE, 100, sizeof(other::Material) },
    };

    uint32_t light_binding_pnt = 3;
    std::vector<other::Uniform> light_unis = {
      { "num_lights", other::ValueType::VEC4 },
      { "point_lights", other::ValueType::USER_TYPE, 100, sizeof(other::PointLight) },
      { "direction_lights", other::ValueType::USER_TYPE, 100, sizeof(other::DirectionLight) },
    };

    Ref<other::UniformBuffer> camera_uniforms = NewRef<other::UniformBuffer>("Camera", camera_unis, camera_binding_pnt);
    camera_uniforms->BindBase();
    Ref<other::UniformBuffer> light_uniforms = NewRef<other::UniformBuffer>("Lights", light_unis, light_binding_pnt, other::SHADER_STORAGE);
    light_uniforms->BindBase();

    Ref<other::UniformBuffer> material_uniforms = NewRef<other::UniformBuffer>("MaterialData", material_unis, material_binding_pnt, other::SHADER_STORAGE);
    material_uniforms->BindBase();
    Ref<other::UniformBuffer> model_uniforms = NewRef<other::UniformBuffer>("ModelData", model_unis, model_binding_pnt, other::SHADER_STORAGE);
    model_uniforms->BindBase();

    other::Buffer model_buffer;
    other::Buffer material_buffer;

    glUseProgram(shader2);
    glUniform1i(glGetUniformLocation(shader2, "g_position"), 0);
    glUniform1i(glGetUniformLocation(shader2, "g_normal"), 1);
    glUniform1i(glGetUniformLocation(shader2, "g_albedo_spec"), 2);
    glUseProgram(0);

    /// generate sampler2DArray
    uint32_t mip_levels = 1;
    glm::ivec2 size = { 800, 600 };
    std::vector<glm::vec3> colors = {
      { 1.f, 0.f, 0.f },
      { 0.f, 1.f, 0.f },
    };
    uint32_t num_colors = colors.size();

    Ref<MaterialTable> material_table = NewRef<MaterialTable>(mip_levels, num_colors, size);
    material_table->SetTexture(MaterialTable::ALBEDO, 0, colors[0]);
    material_table->SetTexture(MaterialTable::ALBEDO, 1, colors[1]);

    material_table->SetTexture(MaterialTable::NORMAL, 0, glm::vec3(1.f));
    material_table->SetTexture(MaterialTable::NORMAL, 1, glm::vec3(1.f));
    material_table->SetTexture(MaterialTable::ROUGHNESS, 0, glm::vec3(1.f));
    material_table->SetTexture(MaterialTable::ROUGHNESS, 1, glm::vec3(1.f));

    OE_DEBUG("Uniforms Set");

    other::GBuffer gbuffer({ win_w, win_h });

    OE_INFO("Running");

    bool running = true;

    bool camera_lock = true;
    bool force_update = true;
    other::Mouse::FreeCursor();

    while (running) {
      other::IO::Update();

      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT: running = false; break;
          case SDL_WINDOWEVENT:
            switch (event.window.event) {
              case SDL_WINDOWEVENT_CLOSE: running = false; break;
              default:
                break;
            }
            break;
          case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
              case SDLK_ESCAPE:
                running = false;
                break;
                break;
              case SDLK_c:
                camera_lock = !camera_lock;
                if (camera_lock) {
                  other::Mouse::FreeCursor();
                } else {
                  other::Mouse::LockCursor();
                }
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }

        ImGui_ImplSDL2_ProcessEvent(&event);
      }
      if (!running) {
        break;
      }

      other::EventQueue::Clear();

      other::Renderer::GetWindow()->Clear();

      /// update camera uniforms
      if (!camera_lock || force_update) {
        force_update = !force_update;
        UpdateCamera(camera);
        proj = camera->ProjectionMatrix();
        view = camera->ViewMatrix();
        glm::vec4 cam_pos = glm::vec4(camera->Position(), 1.f);

        camera_uniforms->SetUniform("projection", camera->ProjectionMatrix());
        camera_uniforms->SetUniform("view", camera->ViewMatrix());
        camera_uniforms->SetUniform("viewpoint", cam_pos);
      }

      light_uniforms->SetUniform("num_lights", glm::vec4{ 0, 1, 0, 0 });
      light_uniforms->SetUniform("point_lights", point_light);
      light_uniforms->SetUniform("direction_lights", dir_light);

      model1 = glm::mat4(1.f);
      model1 = glm::rotate(model1, m1_rotation, { 1.f, 1.f, 1.f });
      m1_rotation += 0.1f;

      // model_buffer.ZeroMem();
      // model_buffer.BufferData(model1);
      // model_buffer.BufferData(model2);

      // model_uniforms->BindBase();
      // model_uniforms->LoadFromBuffer(model_buffer);

      model_buffer.ZeroMem();
      model_buffer.BufferData(model1);
      model_buffer.BufferData(model2);

      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);

      /// start material textures at the the end of gbuffer textures
      material_table->Bind(GBuffer::NUM_TEX_IDXS);
      gbuffer.SetInput("albedo_textures", GBuffer::NUM_TEX_IDXS);
      gbuffer.SetInput("normal_textures", GBuffer::NUM_TEX_IDXS + 1);
      gbuffer.SetInput("roughness_textures", GBuffer::NUM_TEX_IDXS + 2);

      ///> GBUFFER RENDER
      gbuffer.Bind();
      cube.Draw(other::TRIANGLES, 2);
      gbuffer.Unbind();
      /// > GBUFFER RENDER

      /// > LIGHTING PASS
      // frame->BindFrame();
      // deferred_shader->Bind();
      // fb_mesh->Draw(other::TRIANGLES);
      // deferred_shader->Unbind();
      // frame->UnbindFrame();
      /// > LIGHTING PASS

      // /// > GEOMETRY PASS
      frame->BindFrame();

      model_buffer.ZeroMem();
      model_buffer.BufferData(model1);
      model_buffer.BufferData(model2);

      model_uniforms->BindBase();
      model_uniforms->LoadFromBuffer(model_buffer);

      mat_shader->Bind();
      mat_shader->SetUniform("albedo_textures", GBuffer::NUM_TEX_IDXS);
      mat_shader->SetUniform("normal_textures", GBuffer::NUM_TEX_IDXS + 1);
      mat_shader->SetUniform("roughness_textures", GBuffer::NUM_TEX_IDXS + 2);

      cube.Draw(other::TRIANGLES, colors.size());
      mat_shader->Unbind();

      frame->UnbindFrame();
      /// > GEOMETRY PASS

      /// > DRAW TO SCREEN
      other::Renderer::DrawFramebufferToWindow(frame);
      /// > DRAW TO SCREEN

#define UI_ENABLED 1
#if UI_ENABLED
      other::UI::BeginFrame();

      if (ImGui::Begin("GBuffer")) {
        RenderItem(gbuffer.textures[0], "Position", ImVec2((float)win_w / 2, (float)win_h / 2));
        RenderItem(gbuffer.textures[1], "Normals", ImVec2((float)win_w / 2, (float)win_h / 2));
        RenderItem(gbuffer.textures[2], "Albedo", ImVec2((float)win_w / 2, (float)win_h / 2));
      }
      ImGui::End();
  #if 0
      if (ImGui::Begin("Test")) {
        ImVec2 curr_win_size = ImGui::GetContentRegionAvail();
        float aspect_ratio = Renderer::GetWindow()->AspectRatio();
        float ui_aspect_ratio = curr_win_size.x / curr_win_size.y;

        glm::vec2 padding = { 0.f, 0.f };
        glm::vec2 size = { curr_win_size.x, curr_win_size.y };
        if (ui_aspect_ratio > aspect_ratio) {
          size.x = curr_win_size.y * aspect_ratio;
          padding = { (curr_win_size.x - size.x) * 0.5f, 0.f };
        } else {
          size.y = curr_win_size.x / aspect_ratio;
          padding = { 0.f, (curr_win_size.y - size.y) * 0.5f };
        }

        ImTextureID tex_id = (void*)(uintptr_t)gbuffer.textures[0];
        ImVec2 img_size = { size.x, size.y };

        ImVec2 cursor_pos = ImGui::GetCursorPos();
        ImGui::SetCursorPos({ padding.x, padding.y });
        ImGui::Image(tex_id, img_size, ImVec2(0, 1), ImVec2(1, 0));
        ImGui::SetCursorPos(cursor_pos);
      }
      ImGui::End();
  #endif
      other::UI::EndFrame();
#endif

      other::Renderer::GetWindow()->SwapBuffers();
    }

    glDeleteProgram(shader1);
    glDeleteProgram(shader2);

    AppState::DetachApplication();
    /// clear event queue from any remaining events and flush one more event loop with no scene
    ///   to ensure we are in a stable state before shutting down
    // EventQueue::Poll();

    PhysicsEngine::Shutdown();
    ScriptEngine::Shutdown();
    UI::Shutdown();
    Renderer::Shutdown();
    AppState::Shutdown();
  } catch (const other::IniException& e) {
    std::cout << "caught ini error : " << e.what() << "\n";
    exit = 1;
  } catch (const std::exception& e) {
    std::cout << "caught std error : " << e.what() << "\n";
    exit = 1;
  } catch (...) {
    std::cout << "unknown error" << "\n";
    exit = 1;
  }
  std::cout << "Exiting\n";
  return exit;
}

uint32_t BuildMaterialTable(uint32_t mip_levels, const glm::vec2& size, const std::vector<glm::vec3>& colors) {
  uint32_t levels = colors.size();

  std::vector<std::vector<uint8_t>> seperate_textures;
  std::vector<uint8_t> data;

  for (uint32_t i = 0; i < levels; i++) {
    std::vector<uint8_t>& tex_data = seperate_textures.emplace_back();
    tex_data.resize(size.x * size.y * 4);

    for (uint32_t j = 0; j < size.x * size.y; j += 4) {
      tex_data[j + 0] = static_cast<uint8_t>(colors[i].r * 255);
      tex_data[j + 1] = static_cast<uint8_t>(colors[i].g * 255);
      tex_data[j + 2] = static_cast<uint8_t>(colors[i].b * 255);
      tex_data[j + 3] = 255;
    }
  }

  for (uint32_t i = 0; i < levels; i++) {
    std::vector<uint8_t>& tex_data = seperate_textures[i];
    data.insert(data.end(), tex_data.begin(), tex_data.end());
  }

  uint32_t mat_table;

  glGenTextures(1, &mat_table);
  glBindTexture(GL_TEXTURE_2D_ARRAY, mat_table);
  glTexStorage3D(GL_TEXTURE_2D_ARRAY, mip_levels, GL_RGBA8, size.x, size.y, levels);
  CHECK();

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  CHECK();

  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, size.x, size.y, levels, other::ChannelType::RGBA, GL_UNSIGNED_BYTE, data.data());
  CHECK();

  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
  return mat_table;
}

Quad::Quad() {
  vertices = {
    /* (-,+,0) */ -1.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    /* (-,-,0) */ -1.0f,
    -1.0f,
    0.0f,
    0.0f,
    0.0f,
    /* (+,+,0) */ 1.0f,
    1.0f,
    0.0f,
    1.0f,
    1.0f,
    /* (+,-,0) */ 1.0f,
    -1.0f,
    0.0f,
    1.0f,
    0.0f,
  };
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);
}

Quad::~Quad() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
}

void Quad::Draw() {
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

// glm::vec3 position
// glm::vec3 normal
// glm::vec3 tangent
// glm::vec3 bitangent
// glm::vec2 uv_coord

Cube::Cube() {
  // clang-format off
  vertices = {
    /* (-,-,+) */
    -1.0f / 2.0f, -1.0f / 2.0f, 1.0f / 2.0f,
    -1.f, -1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 1.f,
    /* (+,-,+) */ 
    1.0f / 2.0f, -1.0f / 2.0f, 1.0f / 2.0f,
    1.f, -1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 1.f,
    /* (+,+,+) */ 
    1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f,
    1.f, 1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 0.f,
    /* (-,+,+) */ 
    -1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f,
    -1.f, 1.f, 1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 0.f,
    /* (-,-,-) */ 
    -1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f,
    -1.f, -1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 1.f,
    /* (+,-,-) */ 
    1.0f / 2.0f, -1.0f / 2.0f, -1.0f / 2.0f,
    1.f, -1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 1.f,
    /* (+,+,-) */ 
    1.0f / 2.0f, 1.0f / 2.0f, -1.0f / 2.0f,
    1.f, 1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    1.f, 0.f,
    /* (-,+,-) */ 
    -1.0f / 2.0f, 1.0f / 2.0f, -1.0f / 2.0f,
    -1.f, 1.f, -1.f,
    0.f, 0.f, 0.f,
    0.f, 0.f, 0.f,
    0.f, 0.f,
  };
  // clang-format on
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  uint32_t stride = 14;

  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(9 * sizeof(float)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(12 * sizeof(float)));

  glBindVertexArray(0);
}

Cube::~Cube() {
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &ebo);
}

void Cube::Draw(other::DrawMode mode, uint32_t instances) {
  glBindVertexArray(vao);

  // glDrawElementsInstancedBaseVertexBaseInstance(mesh_key.draw_mode, msl.num_elements, GL_UNSIGNED_INT, (void*)0, msl.instance_count, msl.base_vertex, msl.base_instance);
  glDrawElementsInstancedBaseVertexBaseInstance(mode, 36, GL_UNSIGNED_INT, (void*)0, instances, 0, 0);
  glBindVertexArray(0);
}

void UpdateCamera(other::Ref<CameraBase>& camera) {
  if (other::Keyboard::Down(other::Keyboard::Key::OE_W)) {
    camera->MoveForward();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_S)) {
    camera->MoveBackward();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_A)) {
    camera->MoveLeft();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_D)) {
    camera->MoveRight();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_SPACE)) {
    camera->MoveUp();
  }
  if (other::Keyboard::Down(other::Keyboard::Key::OE_LSHIFT)) {
    camera->MoveDown();
  }

  glm::vec2 win_size = { win_w, win_h };
  glm::ivec2 mouse_pos = other::Mouse::GetPos();

  SDL_WarpMouseInWindow(SDL_GetMouseFocus(), win_size.x / 2, win_size.y / 2);

  camera->SetLastMouse(camera->Mouse());
  camera->SetMousePos(mouse_pos);
  camera->SetDeltaMouse({ camera->Mouse().x - camera->LastMouse().x,
                          camera->LastMouse().y - camera->Mouse().y });

  glm::ivec2 rel_pos = other::Mouse::GetRelPos();

  camera->SetYaw(camera->Yaw() + (rel_pos.x * camera->Sensitivity()));
  camera->SetPitch(camera->Pitch() - (rel_pos.y * camera->Sensitivity()));

  if (camera->ConstrainPitch()) {
    if (camera->Pitch() > 89.0f) {
      camera->SetPitch(89.0f);
    }

    if (camera->Pitch() < -89.0f) {
      camera->SetPitch(-89.0f);
    }
  }

  camera->UpdateCoordinateFrame();
  camera->CalculateMatrix();
}
