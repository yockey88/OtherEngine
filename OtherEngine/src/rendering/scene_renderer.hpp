/**
 * \file rendering/scene_renderer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_RENDERER_HPp
#define OTHER_ENGINE_SCENE_RENDERER_HPp

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/pipeline.hpp"

namespace other {

  class SceneRenderer : public RefCounted {
    public:
      SceneRenderer(SceneRenderSpec spec = SceneRenderSpec());
      virtual ~SceneRenderer() override;

      void SetViewportSize(const glm::ivec2& size);
      
      void SubmitModel(Ref<Model> model , const glm::mat4& transform = glm::mat4(1.f));
      void SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform = glm::mat4(1.f));

      void BeginScene(Ref<CameraBase>& camera);
      void EndScene();

    private:
      SceneRenderSpec spec;

      Ref<RenderPass> geometry_pass;
      std::vector<Ref<Pipeline>> pipelines;

      void Initialize();
      void Shutdown();
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_RENDERER_HPp

