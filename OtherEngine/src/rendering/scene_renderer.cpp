/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/geometry_pass.hpp"
#include "rendering/material.hpp"
#include "rendering/model_factory.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/uniform.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec)
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  }

  const std::map<UUID, Ref<Pipeline>>& SceneRenderer::GetPipelines() const {
    return pipelines;
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
  }

  void SceneRenderer::SubmitCamera(Ref<CameraBase>& camera) {
    /// FIXME: different pipelines should be able to have different camera bindings
    if (frame_data.viewpoint != nullptr) {
      /// only one viewpoint per frame
      return;
    }
    OE_ASSERT(camera != nullptr, "Camera is null");
    OE_ASSERT(frame_data.camera_uniforms != nullptr, "Camera uniforms are null");

    const glm::mat4& proj = camera->ProjectionMatrix();
    const glm::mat4& view = camera->ViewMatrix();
    const glm::mat4& inverse_mvp = glm::inverse(proj * view);
    glm::vec4 cam_pos = glm::vec4(camera->Position(), 1.f);

    frame_data.camera_uniforms->SetUniform("projection", proj);
    frame_data.camera_uniforms->SetUniform("view", view);
    frame_data.camera_uniforms->SetUniform("inverse_mvp", inverse_mvp);
    frame_data.camera_uniforms->SetUniform("viewpoint", cam_pos);
    frame_data.viewpoint = camera;
  }

  void SceneRenderer::SubmitEnvironment(Ref<LightEnvironment>& environment) {
    if (frame_data.environment != nullptr) {
      /// only one environment per frame
      return;
    }
    OE_ASSERT(environment != nullptr, "Environment is null");
    OE_ASSERT(frame_data.light_uniforms != nullptr, "Light uniforms are null");

    float num_point_lights = environment->point_lights.size();
    glm::vec4 light_count{
      num_point_lights,
      0, 0, 0
    };
    frame_data.light_uniforms->BindBase();
    frame_data.light_uniforms->SetUniform("num_lights", light_count);
    for (size_t i = 0; i < num_point_lights; ++i) {
      auto& l = environment->point_lights[i];
      frame_data.light_uniforms->SetUniform("point_lights", l, i);
    }

    if (environment->direction_light.has_value()) {
      frame_data.light_uniforms->SetUniform("direction_light", environment->direction_light.value());
    }
    frame_data.environment = environment;
  }

  void SceneRenderer::ClearLightEnvironment() {
    glm::vec4 light_count{ 0, 0, 0, 0 };
    frame_data.light_uniforms->BindBase();
    frame_data.light_uniforms->SetUniform("num_lights", light_count);
  }

  void SceneRenderer::SubmitModel(const Ref<Model>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    if (model == nullptr) {
      return;
    }

    SubmitModel({
      .model = model,
      .transform = transform,
      .material_table = mat_table,
      .material = material_id,
      .draw_mode = topology,
    });
  }

  void SceneRenderer::SubmitModel(const RenderSubmission& submission) {
    if (submission.model == nullptr) {
      return;
    }
    for (auto& [id, pl] : pipelines) {
      pl->SubmitModel(submission);
    }
  }

  void SceneRenderer::SubmitStaticModel(const Ref<StaticModel>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    if (model == nullptr) {
      return;
    }

    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material_table = mat_table,
      .material = material_id,
      .draw_mode = topology,
    });
  }

  void SceneRenderer::SubmitStaticModel(const RenderStaticSubmission& submission) {
    if (submission.model == nullptr) {
      return;
    }
    for (auto& [id, pl] : pipelines) {
      pl->SubmitStaticModel(submission);
    }
  }

  void SceneRenderer::SubmitModel(const std::vector<std::string>& pls, const Ref<Model>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    for (auto& pl : pls) {
      auto itr = pipelines.find(FNV(pl));
      if (itr == pipelines.end()) {
        continue;
      }

      itr->second->SubmitModel(model, mat_table, transform, material_id, topology);
    }
  }

  void SceneRenderer::SubmitStaticModel(const std::vector<std::string>& pls, const Ref<StaticModel>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    for (auto& pl : pls) {
      auto itr = pipelines.find(FNV(pl));
      if (itr == pipelines.end()) {
        continue;
      }

      SubmitStaticModel(pls, {
                               .model = model,
                               .transform = transform,
                               .material_table = mat_table,
                               .material = material_id,
                               .draw_mode = topology,
                             });
    }
  }

  void SceneRenderer::SubmitModel(const std::vector<std::string>& pls, const RenderSubmission& submission) {
    for (auto& pl : pls) {
      auto itr = pipelines.find(FNV(pl));
      if (itr == pipelines.end()) {
        continue;
      }

      itr->second->SubmitModel(submission);
    }
  }

  void SceneRenderer::SubmitStaticModel(const std::vector<std::string>& pls, const RenderStaticSubmission& submission) {
    for (auto& pl : pls) {
      auto itr = pipelines.find(FNV(pl));
      if (itr == pipelines.end()) {
        continue;
      }

      itr->second->SubmitStaticModel(submission);
    }
  }

  void SceneRenderer::SubmitDebugDrawCommands(const std::string_view pl, const std::vector<DebugDrawCommand>& cmds) {
    auto itr = pipelines.find(FNV(pl));
    if (itr == pipelines.end()) {
      return;
    }

    itr->second->SubmitDebugDrawCommands(cmds);
  }

  // void SceneRenderer::DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color, float thickness) {
  //   AssetHandle line_handle = ModelFactory::CreateLine(start, end);
  //   Ref<StaticModel> line = AssetManager::GetAsset<StaticModel>(line_handle);

  //   if (frame_data.material_table == nullptr) {
  //     frame_data.material_table = NewRef<MaterialTable>(glm::ivec2{ 1080, 720 });
  //   }

  //   UUID material_id = frame_data.material_table->RegisterMaterial(color, glm::vec4(1.f), glm::vec4(1.f));

  //   pipelines[FNV("Geometry")]->SubmitStaticModel({
  //     .model = line,
  //     .transform = glm::mat4(1.f),
  //     .material_table = frame_data.material_table,
  //     .material = material_id,
  //     .draw_mode = DrawMode::LINES,
  //     .line_thickness = thickness,
  //   });
  // }

  // void SceneRenderer::DrawRect(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color) {
  //   AssetHandle rect_handle = ModelFactory::CreateRect();
  //   Ref<StaticModel> rect = AssetManager::GetAsset<StaticModel>(rect_handle);

  //   glm::vec3 mid = (min + max) / 2.f;
  //   glm::vec3 scale = glm::abs(max - min) / 2.f;

  //   glm::mat4 transform = glm::translate(glm::mat4(1.f), mid) * glm::scale(glm::mat4(1.f), scale);

  //   if (frame_data.material_table == nullptr) {
  //     frame_data.material_table = NewRef<MaterialTable>(glm::ivec2{ 1080, 720 });
  //   }

  //   UUID material_id = frame_data.material_table->RegisterMaterial(color, glm::vec4(1.f), glm::vec4(1.f));

  //   pipelines[FNV("Geometry")]->SubmitStaticModel({
  //     .model = rect,
  //     .transform = transform,
  //     .material_table = frame_data.material_table,
  //     .material = material_id,
  //     .draw_mode = DrawMode::TRIANGLES,
  //   });
  // }

  // void SceneRenderer::DrawTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color) {
  //   if (a == b || a == c || b == c ||
  //       glm::length(a - b) < 0.0001f || glm::length(a - c) < 0.0001f || glm::length(b - c) < 0.0001f) {
  //     return;
  //   }
  //   AssetHandle triangle_handle = ModelFactory::CreateTriangle();
  //   Ref<StaticModel> triangle = AssetManager::GetAsset<StaticModel>(triangle_handle);

  //   glm::vec3 mod = (a + b + c) / 3.f;
  //   glm::vec3 scale = glm::abs(a - mod);

  //   glm::mat4 transform = glm::mat4(1.f);  // glm::translate(glm::mat4(1.f), mod) * glm::scale(glm::mat4(1.f), scale);

  //   if (frame_data.material_table == nullptr) {
  //     frame_data.material_table = NewRef<MaterialTable>(glm::ivec2{ 1080, 720 });
  //   }

  //   UUID material_id = frame_data.material_table->RegisterMaterial(color, glm::vec4(1.f), glm::vec4(1.f));

  //   pipelines[FNV("Geometry")]->SubmitStaticModel({
  //     .model = triangle,
  //     .transform = transform,
  //     .material_table = frame_data.material_table,
  //     .material = material_id,
  //     .draw_mode = DrawMode::TRIANGLES,
  //   });
  // }

  bool SceneRenderer::Render() {
    if (!FrameComplete()) {
      return false;
    }

    /** Passes to implement
     * ----------------
     * shadow mapping (expensive) :
     *    X shadow map pass
     *  spot shadow map pass
     *
     * pre depth pass
     * hzb compute
     * pre integration
     * light culling
     * skybox pass
     * geometry pass
     *
     * if GTAO passes:
     *  GTAO compute
     *  GTAO denoise compute
     *  AO Composite
     *
     * pre convolution compute
     *
     * if jump flood:
     *  jump flood
     *
     * if SSR pases:
     *  ssr compute
     *  ssr composite
     *
     * if edge detection passes:
     *  edge detection
     *
     * bloom compute
     * composite pass
     **/

    /// FIXME: This whole function needs to be handled better

    Ref<Pipeline>& shadow_map_pl = pipelines[FNV("ShadowMap")];
    Ref<Pipeline>& depth_pl = pipelines[FNV("Depth")];

    glCullFace(GL_FRONT);
    shadow_map_pl->Render();
    glCullFace(GL_BACK);

    depth_pl->Render();

    image_ir[FNV("ShadowMap")] = framebuffers[SHADOW_MAP_FB] = shadow_map_pl->GetOutput();
    image_ir[FNV("Depth")] = framebuffers[DEPTH_TEXTURE_FB] = depth_pl->GetOutput();

    /// gbuffer takes 0-3 (position, normals, albedo, specular) so we start at 4
    /// mat table goes 0-2, so we start at 3
    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, framebuffers[SHADOW_MAP_FB]->texture);
    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, framebuffers[DEPTH_TEXTURE_FB]->texture);

    /// materials go 6-8

    pipelines[FNV("Geometry")]->Render();
    image_ir[FNV("Geometry")] = pipelines[FNV("Geometry")]->GetOutput();

    ResetFrame();
    return true;
  }

  void SceneRenderer::Clear() {
    /// dont clear the mesh key for a tiny optimization on future submissions
    for (auto& [mk, pl] : pipelines) {
      pl->Clear();
    }
  }

  Ref<Framebuffer> SceneRenderer::GetRender(UUID pipeline_id) const {
    auto itr = pipelines.find(pipeline_id);
    if (itr == pipelines.end()) {
      return nullptr;
    }

    return itr->second->GetOutput();
  }

  SceneRenderer::FrameSubmissions::~FrameSubmissions() {
    viewpoint = nullptr;
    environment = nullptr;
    camera_uniforms = nullptr;
    light_uniforms = nullptr;
  }

  void SceneRenderer::Initialize() {
    Ref<Directory> shader_dir = Filesystem::GetDirectory("shaders");
    Ref<FileHandle> shadow_map_shader_file = shader_dir->GetFile("shadow_map.oshader");
    Ref<FileHandle> depth_shader_file = shader_dir->GetFile("depth_shader.oshader");
    OE_ASSERT(shadow_map_shader_file != nullptr, "Failed to get shader file : {}", "shadow_map.oshader");
    OE_ASSERT(depth_shader_file != nullptr, "Failed to get shader file : {}", "depth_shader.oshader");

    Ref<Directory> core_shaders = Filesystem::GetDirectory("core-shaders");
    OE_ASSERT(core_shaders != nullptr, "Failed to get core shaders directory");
    OE_ASSERT(core_shaders->Exists(), "Core shaders directory does not exist");

    Ref<FileHandle> default_shader_file = core_shaders->GetFile("default.oshader");
    OE_ASSERT(default_shader_file != nullptr, "Failed to get default shader file : {}", "default.oshader");
    OE_ASSERT(default_shader_file->Exists(), "Default shader file does not exist : {}", "default.oshader");

    Ref<Shader> default_shader = AssetManager::GetAsset<Shader>(default_shader_file->handle, default_shader_file->GetAssetType());
    OE_ASSERT(default_shader != nullptr, "Failed to get default shader : {}", "default.oshader");

    Ref<Shader> shadow_map_shader = AssetManager::GetAsset<Shader>(shadow_map_shader_file->handle, shadow_map_shader_file->GetAssetType());
    RenderPassSpec shadow_map_spec = {
      .name = "shadow-map-pass",
      .tag_col = { 0.f, 0.f, 0.f, 1.f },
      .shader = shadow_map_shader,
    };

    Ref<Shader> depth_shader = AssetManager::GetAsset<Shader>(depth_shader_file->handle, depth_shader_file->GetAssetType());
    RenderPassSpec depth_spec = {
      .name = "depth-pass",
      .tag_col = { 1.f, 0.f, 0.f, 1.f },
      .shader = depth_shader,
    };

    render_passes[SHADOW_MAP] = NewRef<RenderPass>(shadow_map_spec);
    render_passes[DEPTH_PASS] = NewRef<RenderPass>(depth_spec);

    std::vector<Uniform> geometry_unis = {};
    render_passes[GEOMETRY_PASS] = NewRef<GeometryPass>(geometry_unis, default_shader);

    uint32_t camera_binding_pnt = spec.camera_binding_pnt;
    std::vector<Uniform> cam_unis = spec.cam_unis.size() > 0 ?
      spec.cam_unis :
      std::vector<Uniform>{
        { "projection", ValueType::MAT4 },
        { "view", ValueType::MAT4 },
        { "inverse_mvp", ValueType::MAT4 },
        { "viewpoint", ValueType::VEC4 },
      };

    uint32_t light_binding_pnt = spec.light_binding_pnt;
    std::vector<Uniform> light_unis = spec.light_unis.size() > 0 ?
      spec.light_unis :
      std::vector<Uniform>{
        { "num_lights", ValueType::VEC4 },
        { "direction_light", ValueType::USER_TYPE, 1, sizeof(DirectionLight) },
        { "point_lights", ValueType::USER_TYPE, 100, sizeof(PointLight) },
      };

    spec.vertex_layout = {
      { ValueType::VEC3, "position" },
      { ValueType::VEC3, "normal" },
      { ValueType::VEC3, "tangent" },
      { ValueType::VEC3, "binormal" },
      { ValueType::VEC2, "uvs" }
    };

    frame_data.camera_uniforms = NewRef<UniformBuffer>("Camera", cam_unis, camera_binding_pnt);
    frame_data.light_uniforms = NewRef<UniformBuffer>("Lights", light_unis, light_binding_pnt, SHADER_STORAGE);

    OE_ASSERT(frame_data.camera_uniforms != nullptr, "Failed to create camera uniforms");
    OE_ASSERT(frame_data.light_uniforms != nullptr, "Failed to create light uniforms");

    frame_data.camera_uniforms->BindBase();
    frame_data.light_uniforms->BindBase();

    /// already made render passes
    for (auto& rp : spec.passes) {
      custom_passes[FNV(rp->Name())] = Ref<RenderPass>::Clone(rp);
    }

    /// pipelines
    for (auto& pl : spec.pipelines) {
      pipelines[FNV(pl.pipeline_name)] = NewRef<Pipeline>(pl);
    }

    for (auto& [id, pass_registration] : spec.pipeline_passes) {
      auto itr = pipelines.find(id);
      if (itr == pipelines.end()) {
        continue;
      }
      auto [_, pipeline] = *itr;

      for (const auto& pass : pass_registration.passes) {
        if (pass >= NUM_RENDER_PASSES) {
          continue;
        }

        pipeline->SubmitRenderPass(render_passes[pass]);
      }
    }

    for (auto& [pipeline_id, pass_list] : spec.pipeline_custom_passes) {
      auto itr = pipelines.find(pipeline_id);
      if (itr == pipelines.end()) {
        continue;
      }
      auto [_, pipeline] = *itr;

      for (auto& pass_id : pass_list) {
        auto pass_itr = custom_passes.find(pass_id);
        if (pass_itr == custom_passes.end()) {
          continue;
        }
        auto [__, pass] = *pass_itr;

        pipeline->SubmitRenderPass(pass);
      }
    }
  }

  void SceneRenderer::Shutdown() {
    frame_data = {};
  }

  void SceneRenderer::PreRenderSettings() {
  }

  bool SceneRenderer::FrameComplete() const {
    return frame_data.viewpoint != nullptr && frame_data.environment != nullptr;
  }

  void SceneRenderer::ResetFrame() {
    frame_data.viewpoint = nullptr;
    frame_data.environment = nullptr;
    frame_data.material_table = nullptr;
  }

}  // namespace other
