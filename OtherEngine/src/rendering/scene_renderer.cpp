/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"
#include "rendering/uniform.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec) 
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  }
      
  void SceneRenderer::ToggleWireframe() {
    render_state = (render_state == WIREFRAME) ?
      FILL : WIREFRAME;
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
  }
  
  
  void SceneRenderer::BeginScene(Ref<CameraBase>& camera /* , Ref<Environment>& lighting */) {
    spec.camera_uniforms->BindBase();
    spec.camera_uniforms->SetUniform("projection" , camera->ProjectionMatrix());
    spec.camera_uniforms->SetUniform("view" , camera->ViewMatrix());

    glm::vec4 cam_pos = glm::vec4(camera->Position() , 1.f);
    spec.camera_uniforms->SetUniform("viewpoint" , cam_pos);
  }
  
  void SceneRenderer::SubmitModel(const std::string_view pl_name , Ref<Model> model , const glm::mat4& transform , const Material& material) {
  }

  void SceneRenderer::SubmitStaticModel(const std::string_view pl_name , Ref<StaticModel> model , 
                                        const glm::mat4& transform , const Material& material) {
    auto hash = FNV(pl_name);
    auto itr = pipelines.find(hash); 
    if (itr == pipelines.end()) {
      return;
    }

    auto& [id , pl] = *itr;
    pl->SubmitStaticModel(model , transform , material);
  }
  
  void SceneRenderer::EndScene() {
    PreRenderSettings();
    FlushDrawList();
  }

  const std::map<UUID , Ref<Framebuffer>>& SceneRenderer::GetRender() const {
    return image_ir;
  }


  void SceneRenderer::Initialize() {

    /// already made render passes
    for (auto& rp : spec.ref_passes) {
      passes[FNV(rp->Name())] = rp;
    }

    /// render passes from specifications
    for (auto& rp : spec.passes) {
      passes[FNV(rp.name)] = NewRef<RenderPass>(rp);
    }

    /// pipelines
    for (auto& pl : spec.pipelines) {
      pipelines[FNV(pl.debug_name)] = NewRef<Pipeline>(pl);
    }

    /// register passes with pipelines
    for (auto& [pl , rps] : spec.pipeline_to_pass_map) {
      /// FIXME: pl might not exist
      auto& pline = pipelines[pl];

      for (auto& p : rps) {
        auto itr = passes.find(p);
        if (itr == passes.end()) {
          continue;
        }
        const auto& pass = itr->second;
        pline->SubmitRenderPass(pass);
      }
    }
  }

  void SceneRenderer::Shutdown() {
    pipelines.clear();
  }
 
  void SceneRenderer::PreRenderSettings() {
    /// TODO: figure out why glPolygonMode causes INVALID ENUM ?? here 
    ///       but not below
    // glPolygonMode(GL_FRONT_AND_BACK , render_state);
    switch (render_state) {
      case POINT:
        glPolygonMode(GL_FRONT_AND_BACK , POINT);
        break;

      case WIREFRAME:
        glPolygonMode(GL_FRONT_AND_BACK , WIREFRAME);
        break;

      case FILL:
      default:
        glPolygonMode(GL_FRONT_AND_BACK , FILL);
        break;
    }
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

    /// TODO: certain pipelines should be rendered before others so we can use pipeline outputs as 
    ///       inputs to other pipelines
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
  
} // namespace other
