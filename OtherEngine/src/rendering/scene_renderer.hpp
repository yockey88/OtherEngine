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
      //// TODO: find way to configure render passes performed! Right now hard coded :(
      SceneRenderer(SceneRenderSpec spec = SceneRenderSpec());
      virtual ~SceneRenderer() override;

      void SetViewportSize(const glm::ivec2& size);
      
      void SubmitModel(Ref<Model> model , const glm::mat4& transform = glm::mat4(1.f));
      void SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform = glm::mat4(1.f));

      // void SubmitEnv(...)
      // void SubmitLighting(...)
      
      Ref<Framebuffer> RenderScene(Ref<CameraBase>& camera);

    private:
      glm::ivec2 viewport_size;
      SceneRenderSpec spec;

      Ref<CameraBase> viewpoint;

      /// here go the passes
      ///  - bloom compute ?
      ///  - directional shadow pass
      ///  - non-directional-shadow mapping pass
      ///  - pre-depth 
      ///  - geometry
      ///  - selected geometry
      ///  - geometry 2 ? 
      ///  - animated geometry
      ///  - light-culling
      ///  - hierarchical z buffer
      ///  - ssr compute
      ///  - pre-integration
      ///  - pre-convolutional compute
      ///  - edge detection
      ///  - composite
      ///  - DOF
      ///  - wireframe
      ///  -  > read back image >
      ///  - temp fbs for reuse
      ///  - jump flood ??
      ///  - outline compositing
      ///  - grid
      ///  - collider
      ///  - skybox

      Ref<RenderPass> geometry_pass;
      std::vector<Ref<Pipeline>> pipelines;
      std::vector<Ref<Framebuffer>> image_ir;

      std::map<MeshKey , Ref<Model>> models;
      std::map<MeshKey , Ref<StaticModel>> static_models;

      void Initialize();
      void Shutdown();

      Ref<Framebuffer> RenderFrame(Ref<Pipeline> pipeline);
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_RENDERER_HPp

