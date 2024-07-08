/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include "core/defines.hpp"
#include "rendering/uniform.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec) 
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
  }
  
  void SceneRenderer::SubmitModel(Ref<Model> model , const glm::mat4& transform) {
  }

  void SceneRenderer::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform) {
    uint32_t idx = GetCurrentTransformIdx();

    spec.model_storage->SetUniform("models" , transform , idx);
    /// decide which pipeline to submit model to
    for (auto& [id , pl] : pipelines) {
      pl->SubmitStaticModel(model , idx);
    }
  }

  /**
   * fn SetUniforms(camera , light)
   *    for u in camera.uniforms 
   *      passes.bind(u)
   *    for l in light
   *      passes.bind(l)
   **/
  void SceneRenderer::BeginScene(Ref<CameraBase>& camera) {
    spec.camera_uniforms->SetUniform("projection" , camera->ProjectionMatrix());
    spec.camera_uniforms->SetUniform("view" , camera->ViewMatrix());
  }
  
  void SceneRenderer::EndScene() {
    FlushDrawList();

    curr_model_idx = 0;
  }

  const std::map<UUID , Ref<Framebuffer>>& SceneRenderer::GetRender() const {
    return image_ir;
  }


  void SceneRenderer::Initialize() {
    spec.camera_uniforms->BindBase();
    spec.model_storage->BindBase();

    for (auto& rp : spec.passes) {
      passes[FNV(rp.name)] = NewRef<RenderPass>(rp);
    }

    for (auto& pl : spec.pipelines) {
      pipelines[FNV(pl.debug_name)] = NewRef<Pipeline>(pl);
    }

    for (auto& [pl , rps] : spec.pipeline_to_pass_map) {
      auto& pline = pipelines[pl];

      for (auto& p : rps) {
        auto& pass = passes[p];
        pline->SubmitRenderPass(pass);
      }
    }
  }

  void SceneRenderer::Shutdown() {
    pipelines.clear();
  }
      
  void SceneRenderer::FlushDrawList() {
    /// pre render
    /// begin command buffer ?
    ///
    /// shadow map pass
    /// spot shadow mapp pass
    /// pre depth pass
    /// hzb compute
    /// pre integration
    /// light culling
    /// skybox pass

    for (auto& [id , pl] : pipelines) {
      pl->Render();
      image_ir[id] = pl->GetOutput();
    }

    /// GTAO compute
    /// GTAO denoise compute
    /// AO Composite
    /// pre convolution compute
    ///
    /// jump flood
    ///
    /// ssr compute
    /// ssr composite
    /// edge detection
    /// bloom compute 
    /// composite pass
    ///
    /// end command buffer
    /// submit command buffer
  }
  
  uint32_t SceneRenderer::GetCurrentTransformIdx() {
    uint32_t idx = curr_model_idx;
    ++curr_model_idx;
    return idx;
  }
      
} // namespace other
