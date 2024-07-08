/**
 * \file rendering/scene_renderer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_RENDERER_HPp
#define OTHER_ENGINE_SCENE_RENDERER_HPp

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/pipeline.hpp"

namespace other {
  
  struct SceneRenderSpec {
    Ref<UniformBuffer> camera_uniforms;
    Ref<UniformBuffer> model_storage;

    std::vector<RenderPassSpec> passes;
    std::vector<PipelineSpec> pipelines;

    std::map<UUID , std::vector<UUID>> pipeline_to_pass_map;
  };

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
      
      void BeginScene(Ref<CameraBase>& camera);
      void EndScene();
      const std::map<UUID , Ref<Framebuffer>>& GetRender() const;

    private:
      glm::ivec2 viewport_size;
      SceneRenderSpec spec;

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
      
      uint32_t curr_model_idx = 0;

      std::map<UUID , Ref<RenderPass>> passes;
      std::map<UUID , Ref<Pipeline>> pipelines;

      std::map<UUID , Ref<Framebuffer>> image_ir;

      void Initialize();
      void Shutdown();

      void FlushDrawList();
      
      uint32_t GetCurrentTransformIdx();
  };

} // namespace other

#endif // !OTHER_ENGINE_SCENE_RENDERER_HPp

