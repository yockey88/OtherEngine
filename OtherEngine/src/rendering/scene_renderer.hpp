/**
 * \file rendering/scene_renderer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_RENDERER_HPp
#define OTHER_ENGINE_SCENE_RENDERER_HPp

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "scene/environment.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/render_pass.hpp"

namespace other {

  struct SceneRenderSpec {
    Ref<UniformBuffer> camera_uniforms;
    Ref<UniformBuffer> light_uniforms;

    std::vector<PipelineSpec> pipelines;
    std::vector<Ref<RenderPass>> passes;

    std::map<UUID, std::vector<UUID>> pipeline_to_pass_map;
  };

  class SceneRenderer : public RefCounted {
   public:
    SceneRenderer(SceneRenderSpec spec = SceneRenderSpec());
    virtual ~SceneRenderer() override;

    template <typename T>
    void SetUniform(const std::string_view pass, const std::string_view block, const std::string_view name, const T& val, uint32_t index = 0) {
      if (auto itr = passes.find(FNV(pass)); itr != passes.end()) {
        itr->second->SetInput(block, name, val, index);
      }
    }

    template <typename T>
    void SetUniform(const std::string_view pass, std::string_view name, const T& val, uint32_t index = 0) {
      if (auto itr = passes.find(FNV(pass)); itr != passes.end()) {
        itr->second->SetInput(name, val, index);
      }
    }

    template <typename T>
    void SetLightUniform(const std::string_view name, const T& val, uint32_t index = 0) {
      spec.light_uniforms->BindBase();
      spec.light_uniforms->SetUniform(name, val, index);
    }

    void SetViewportSize(const glm::ivec2& size);

    void SubmitCamera(const Ref<CameraBase>& camera);
    void SubmitEnvironment(const Ref<Environment>& environment);

    void SubmitDirectionLight(const DirectionLight& light);
    void SubmitPointLight(const PointLight& light);

    void SubmitModel(const std::string_view pl_name, Ref<Model> model, const glm::mat4& transform, const Material& material);
    void SubmitStaticModel(const std::string_view pl_name, Ref<StaticModel> model, const glm::mat4& transform, const Material& material);
    void SubmitStaticModel(const std::string_view pl_name, const RenderSubmission& submission);

    bool EndScene();

    void ClearPipelines();

    const std::map<UUID, Ref<Framebuffer>>& GetRender() const;

   private:
    glm::ivec2 viewport_size;
    SceneRenderSpec spec;

    uint32_t gbuffer = 0;

    enum GBufferTextureType {
      POSITION = 0,
      NORMALS,
      ALBEDO,
      SPECULAR,

      NUM_GBUFFER_TEXTURES,
    };
    uint32_t gbuffer_textures[NUM_GBUFFER_TEXTURES] = {
      0, 0, 0, 0};

    struct FrameSubmissions {
      Ref<CameraBase> viewpoint = nullptr;
      Ref<Environment> environment = nullptr;
    } frame_data;

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

    std::map<UUID, Ref<RenderPass>> passes;
    std::map<UUID, Ref<Pipeline>> pipelines;

    std::map<UUID, Ref<Framebuffer>> image_ir;

    void Initialize();
    void Shutdown();

    void PreRenderSettings();
    void FlushDrawList();

    bool FrameComplete() const;
    void ResetFrame();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_RENDERER_HPp
