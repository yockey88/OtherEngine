/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec) 
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
  }
  
  void SceneRenderer::SubmitModel(Ref<Model> model , const glm::mat4& transform) {}

  void SceneRenderer::SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform) {}

  void SceneRenderer::BeginScene(Ref<CameraBase>& camera) {
  }

  void SceneRenderer::EndScene() {
  }

  void SceneRenderer::Initialize() {
    for (const auto& s : spec.pipeline_descs) {
      pipelines.emplace_back(Ref<Pipeline>::Create(s));
    }   
  }

  void SceneRenderer::Shutdown() {
    pipelines.clear();
  }

} // namespace other
