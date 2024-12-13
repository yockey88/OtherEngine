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

    std::vector<Uniform> model_uniforms{};
    uint32_t model_binding_point = 1;

    std::vector<Uniform> material_uniforms{};
    uint32_t material_binding_point = 2;

    uint32_t light_binding_pnt = 3;
    std::vector<Uniform> light_unis = {};

    Layout vertex_layout;

    std::vector<PipelineSpec> pipelines;
    std::vector<Ref<RenderPass>> passes;

    std::map<UUID, std::vector<UUID>> pipeline_to_pass_map;
  };

  class SceneRenderer : public RefCounted {
   public:
    SceneRenderer(SceneRenderSpec spec = SceneRenderSpec());
    virtual ~SceneRenderer() override;

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

    void SubmitCamera(Ref<CameraBase>& camera);
    void SubmitEnvironment(Ref<LightEnvironment>& environment);

    void ClearLightEnvironment();

    void SubmitDirectionLight(const DirectionLight& light);
    void SubmitPointLight(const PointLight& light);

    void SubmitModel(Ref<Model> model, const glm::mat4& transform, const Material& material, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitStaticModel(Ref<StaticModel> model, const glm::mat4& transform, const Material& material, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitStaticModel(const RenderSubmission& submission);

    void RenderGbuffer();
    bool RenderAll();
    bool FinalizeScene();

    void Clear();

    const Ref<Framebuffer>& GetRender() const;

    struct Framebuffers {
      Ref<Framebuffer> shadow_map = nullptr;
      Ref<Framebuffer> depth_texture = nullptr;
      Ref<Framebuffer> final_frame = nullptr;
    } framebuffers;

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

    struct Passes {
      Ref<RenderPass> shadow_map_pass = nullptr;
      Ref<RenderPass> depth_pass = nullptr;
      Ref<RenderPass> geometry_pass = nullptr;
    } render_passes;

    glm::ivec2 viewport_size;
    SceneRenderSpec spec;

    Ref<GBuffer> gbuffer = nullptr;
    FrameMeshes model_submissions;

    Ref<UniformBuffer> camera_uniforms = nullptr;
    Ref<UniformBuffer> light_uniforms = nullptr;
    Ref<UniformBuffer> model_storage = nullptr;
    Ref<UniformBuffer> material_storage = nullptr;

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

    void Initialize();
    void Shutdown();

    void PreRenderSettings();
    void Render();

    void PerformDepthPass();

    void PerformPass(Ref<RenderPass>& pass);
    void RenderMeshList(const MeshKey& mesh_key, uint32_t instance_count, const Buffer& model_buffer, const Buffer& material_buffer);

    bool FrameComplete() const;
    void ResetFrame();

    FrameMeshes::iterator InsertMeshKey(MeshKey& key, const std::vector<float>& vertices, const std::vector<Index>& indices);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_RENDERER_HPp
