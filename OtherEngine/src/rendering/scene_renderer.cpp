/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"
#include "core/logger.hpp"

#include "rendering/uniform.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec)
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
    for (auto& [_, pl] : pipelines) {
      pl->SetViewportSize(size);
    }
  }

  void SceneRenderer::SubmitCamera(const Ref<CameraBase>& camera) {
    if (frame_data.viewpoint != nullptr) {
      /// only one viewpoint per frame
      return;
    }
    OE_ASSERT(camera != nullptr, "Camera is null");
    OE_ASSERT(camera_uniforms != nullptr, "Camera uniforms are null");

    const glm::mat4& proj = camera->ProjectionMatrix();
    const glm::mat4& view = camera->ViewMatrix();
    glm::vec4 cam_pos = glm::vec4(camera->Position(), 1.f);

    camera_uniforms->SetUniform("projection", proj);
    camera_uniforms->SetUniform("view", view);
    camera_uniforms->SetUniform("viewpoint", cam_pos);
    frame_data.viewpoint = camera;
  }

  void SceneRenderer::SubmitEnvironment(const Ref<LightEnvironment>& environment) {
    if (frame_data.environment != nullptr) {
      /// only one environment per frame
      return;
    }
    OE_ASSERT(environment != nullptr, "Environment is null");
    OE_ASSERT(light_uniforms != nullptr, "Light uniforms are null");

    float num_dir_lights = environment->direction_lights.size();
    float num_point_lights = environment->point_lights.size();
    glm::vec4 light_count{
      num_dir_lights, num_point_lights,
      0, 0
    };
    light_uniforms->BindBase();
    light_uniforms->SetUniform("num_lights", light_count);
    for (size_t i = 0; i < num_point_lights; ++i) {
      auto& l = environment->point_lights[i];
      light_uniforms->SetUniform("point_lights", l, i);
    }
    for (size_t i = 0; i < num_dir_lights; ++i) {
      auto& l = environment->direction_lights[i];
      light_uniforms->SetUniform("direction_lights", l, i);
    }
    frame_data.environment = environment;
  }

  void SceneRenderer::SubmitModel(const std::string_view pl_name, Ref<Model> model, const glm::mat4& transform, const Material& material) {
    if (model == nullptr) {
      return;
    }
  }

  void SceneRenderer::SubmitStaticModel(const std::string_view pl_name, Ref<StaticModel> model, const glm::mat4& transform, const Material& material) {
    if (model == nullptr) {
      return;
    }

    auto itr = pipelines.find(FNV(pl_name));
    if (itr == pipelines.end()) {
      OE_ERROR("Submitting model to unknown pipeline {}!", pl_name);
      return;
    }

    itr->second->SubmitStaticModel(model, transform, material);
  }

  void SceneRenderer::SubmitStaticModel(const std::string_view pl_name, const RenderSubmission& submission) {
    if (submission.model == nullptr) {
      return;
    }

    auto itr = pipelines.find(FNV(pl_name));
    if (itr == pipelines.end()) {
      OE_ERROR("Submitting model to unknown pipeline {}!", pl_name);
      return;
    }

    itr->second->SubmitStaticModel(submission);
  }

  bool SceneRenderer::EndScene() {
    if (!FrameComplete()) {
      ResetFrame();
      return false;
    }

    PreRenderSettings();
    FlushDrawList();
    ResetFrame();
    return true;
  }

  void SceneRenderer::ClearPipelines() {
    for (auto& [id, pl] : pipelines) {
      pl->Clear();
    }
  }

  const std::map<UUID, Ref<Framebuffer>>& SceneRenderer::GetRender() const {
    return image_ir;
  }

  void SceneRenderer::Initialize() {
    /// already made render passes
    for (auto& rp : spec.passes) {
      passes[FNV(rp->Name())] = Ref<RenderPass>::Clone(rp);
    }

    /// pipelines
    for (auto& pl : spec.pipelines) {
      pipelines[FNV(pl.debug_name)] = NewRef<Pipeline>(pl);
    }

    for (auto& [pipeline_id, pass_list] : spec.pipeline_to_pass_map) {
      auto itr = pipelines.find(pipeline_id);
      if (itr == pipelines.end()) {
        continue;
      }
      auto [_, pipeline] = *itr;

      for (auto& pass_id : pass_list) {
        auto pass_itr = passes.find(pass_id);
        if (pass_itr == passes.end()) {
          continue;
        }
        auto [__, pass] = *pass_itr;

        pipeline->SubmitRenderPass(pass);
      }
    }

    uint32_t camera_binding_pnt = spec.camera_binding_pnt;
    std::vector<Uniform> cam_unis = spec.cam_unis.size() > 0 ?
      spec.cam_unis :
      std::vector<Uniform>{
        { "projection", ValueType::MAT4 },
        { "view", ValueType::MAT4 },
        { "viewpoint", ValueType::VEC4 },
      };

    uint32_t light_binding_pnt = spec.light_binding_pnt;
    std::vector<Uniform> light_unis = spec.light_unis.size() > 0 ?
      spec.light_unis :
      std::vector<Uniform>{
        { "num_lights", ValueType::VEC4 },
        { "point_lights", ValueType::USER_TYPE, 100, sizeof(PointLight) },
        { "direction_lights", ValueType::USER_TYPE, 100, sizeof(DirectionLight) },
      };

    camera_uniforms = NewRef<UniformBuffer>("Camera", cam_unis, camera_binding_pnt);
    light_uniforms = NewRef<UniformBuffer>("Lights", light_unis, light_binding_pnt, SHADER_STORAGE);
    OE_ASSERT(camera_uniforms != nullptr, "Failed to create camera uniforms");
    OE_ASSERT(light_uniforms != nullptr, "Failed to create light uniforms");

    camera_uniforms->BindBase();
    light_uniforms->BindBase();
  }

  void SceneRenderer::Shutdown() {
    pipelines.clear();
  }

  void SceneRenderer::PreRenderSettings() {
    /// TODO: figure out why glPolygonMode causes INVALID ENUM ?? here
    ///       but not below
  }

  void SceneRenderer::FlushDrawList() {
    for (auto& [id, pl] : pipelines) {
      pl->Render();
      image_ir[id] = pl->GetOutput();
    }
  }

  bool SceneRenderer::FrameComplete() const {
    return frame_data.viewpoint != nullptr &&
      frame_data.environment != nullptr;
  }

  void SceneRenderer::ResetFrame() {
    frame_data.viewpoint = nullptr;
    frame_data.environment = nullptr;
  }

}  // namespace other
