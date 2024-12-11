/**
 * \file rendering/scene_renderer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_RENDERER_HPp
#define OTHER_ENGINE_SCENE_RENDERER_HPp

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "scene/light_environment.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/model.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/render_pass.hpp"

namespace other {

  struct SceneRenderSpec {
    uint32_t camera_binding_pnt = 0;
    std::vector<Uniform> cam_unis = {};

    uint32_t light_binding_pnt = 3;
    std::vector<Uniform> light_unis = {};

    std::vector<PipelineSpec> pipelines;
    std::vector<Ref<RenderPass>> passes;

    std::map<UUID, std::vector<UUID>> pipeline_to_pass_map;
  };

  class SceneRenderer : public RefCounted {
   public:
    SceneRenderer(SceneRenderSpec spec = SceneRenderSpec());
    virtual ~SceneRenderer() override;

    void AddPipeline(PipelineSpec& spec);
    void AddRenderPass(RenderPassSpec& spec);

    const std::map<UUID, Ref<Pipeline>>& GetPipelines() const;
    const std::map<UUID, Ref<RenderPass>>& GetRenderPasses() const;

    void AttachPassToPipeline(UUID pipeline, UUID pass);

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
      light_uniforms->BindBase();
      light_uniforms->SetUniform(name, val, index);
    }

    template <typename T>
    void SetCameraUniform(const std::string_view name, const T& val, uint32_t index = 0) {
      camera_uniforms->BindBase();
      camera_uniforms->SetUniform(name, val, index);
    }

    void SetViewportSize(const glm::ivec2& size);

    void SubmitCamera(const Ref<CameraBase>& camera);
    void SubmitEnvironment(const Ref<LightEnvironment>& environment);

    void ClearLightEnvironment();

    void SubmitDirectionLight(const DirectionLight& light);
    void SubmitPointLight(const PointLight& light);

    void SubmitModel(const std::string_view pl_name, Ref<Model> model, const glm::mat4& transform, const Material& material);
    void SubmitStaticModel(const std::string_view pl_name, Ref<StaticModel> model, const glm::mat4& transform, const Material& material);
    void SubmitStaticModel(const std::string_view pl_name, const RenderSubmission& submission);

    void RenderGbuffer();

    bool RenderAll();
    /// feed-forward outputs into input frames

    /// verify we have fed-forward all outputs
    /// render to final framebuffer
    bool FinalizeScene();

    void ClearPipelines();

    const std::map<UUID, Ref<Framebuffer>>& GetRender() const;

   private:
    enum GBufferTextureType {
      POSITION = 0,
      NORMALS,
      ALBEDO,
      SPECULAR,

      NUM_GBUFFER_TEXTURES,
    };

    struct FrameSubmissions {
      Ref<CameraBase> viewpoint = nullptr;
      Ref<LightEnvironment> environment = nullptr;
    } frame_data;

    glm::ivec2 viewport_size;
    SceneRenderSpec spec;

    Ref<UniformBuffer> camera_uniforms = nullptr;
    Ref<UniformBuffer> light_uniforms = nullptr;

    uint32_t gbuffer = 0;

    uint32_t gbuffer_textures[NUM_GBUFFER_TEXTURES] = { 0, 0, 0, 0 };

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
