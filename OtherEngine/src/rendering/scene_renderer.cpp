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
  
  void SceneRenderer::SubmitCamera(const Ref<CameraBase>& camera) {
    if (frame_data.viewpoint != nullptr) {
      /// only one viewpoint per frame
      return;
    }

    spec.camera_uniforms->BindBase();
    spec.camera_uniforms->SetUniform("projection" , camera->ProjectionMatrix());
    spec.camera_uniforms->SetUniform("view" , camera->ViewMatrix());
    
    glm::vec4 cam_pos = glm::vec4(camera->Position() , 1.f);
    spec.camera_uniforms->SetUniform("viewpoint" , cam_pos);

    frame_data.viewpoint = camera;
  }
  
  void SceneRenderer::SubmitEnvironment(const Ref<Environment>& environment) {
    if (frame_data.environment != nullptr) {
      /// only one environment per frame
      return;
    }

    // glm::vec4 light_count{ 
    //   environment->direction_lights.size() , 
    //   environment->point_lights.size() ,
    //   0 , 0
    // };
    // spec.light_uniforms->BindBase();
    // spec.light_uniforms->SetUniform("num_lights" , light_count);
    // for (size_t i = 0; i < environment->direction_lights.size(); ++i) {
    //   auto& l = environment->direction_lights[i];
    //   spec.light_uniforms->SetUniform("direction_lights" , l , i); 
    // }

    // for (size_t i = 0; i < environment->point_lights.size(); ++i) {
    //   auto& l = environment->point_lights[i];
    //   spec.light_uniforms->SetUniform("point_lights" , l , i); 
    // }
    frame_data.environment = environment;
  }
  
  void SceneRenderer::SubmitModel(const std::string_view pl_name , Ref<Model> model , const glm::mat4& transform , const Material& material) {
  }

  void SceneRenderer::SubmitStaticModel(const std::string_view pl_name , Ref<StaticModel> model , 
                                        const glm::mat4& transform , const Material& material) {
    auto itr = pipelines.find(FNV(pl_name)); 
    if (itr == pipelines.end()) {
      return;
    }

    auto& [id , pl] = *itr;
    pl->SubmitStaticModel(model , transform , material);
  }
  
  void SceneRenderer::EndScene() {
    if (!FrameComplete()) {
      ResetFrame();
      return;
    }

    PreRenderSettings();
    FlushDrawList();
    ResetFrame();
  }

  const std::map<UUID , Ref<Framebuffer>>& SceneRenderer::GetRender() const {
    return image_ir;
  }


  void SceneRenderer::Initialize() {

    /// already made render passes
    for (auto& rp : spec.ref_passes) {
      passes[FNV(rp->Name())] = rp;
    }

    /// pipelines
    for (auto& pl : spec.pipelines) {
      pipelines[FNV(pl.debug_name)] = NewRef<Pipeline>(pl);
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
    for (auto& [id , pl] : pipelines) {
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
  
} // namespace other
