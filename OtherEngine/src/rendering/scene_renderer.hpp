/**
 * \file rendering/scene_renderer.hpp
 **/
#ifndef OTHER_ENGINE_SCENE_RENDERER_HPp
#define OTHER_ENGINE_SCENE_RENDERER_HPp

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "scene/light_environment.hpp"

#include "rendering/camera_base.hpp"
#include "rendering/draw_calls.hpp"
#include "rendering/gbuffer.hpp"
#include "rendering/model.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/render_pass.hpp"

namespace other {

  struct PipelineRenderPassRegistration;

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

    std::map<UUID, std::vector<UUID>> pipeline_custom_passes;
    std::map<UUID, PipelineRenderPassRegistration> pipeline_passes;
  };

  class SceneRenderer : public RefCounted {
   public:
    SceneRenderer(SceneRenderSpec spec);
    virtual ~SceneRenderer() override;

    const std::map<UUID, Ref<Pipeline>>& GetPipelines() const;

    template <typename T>
    void SetLightUniform(const std::string_view name, const T& val, uint32_t index = 0) {
      OE_ASSERT(frame_data.light_uniforms != nullptr, "Light uniforms are null");
      frame_data.light_uniforms->BindBase();
      frame_data.light_uniforms->SetUniform(name, val, index);
    }

    template <typename T>
    void SetCameraUniform(const std::string_view name, const T& val, uint32_t index = 0) {
      OE_ASSERT(frame_data.camera_uniforms != nullptr, "Camera uniforms are null");
      frame_data.camera_uniforms->BindBase();
      frame_data.camera_uniforms->SetUniform(name, val, index);
    }

    void SetViewportSize(const glm::ivec2& size);

    void SubmitCamera(Ref<CameraBase>& camera);
    void SubmitEnvironment(Ref<LightEnvironment>& environment);

    void ClearLightEnvironment();

    void SubmitDirectionLight(const DirectionLight& light);
    void SubmitPointLight(const PointLight& light);

    void SubmitModel(const Ref<Model>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitModel(const RenderSubmission& submission);

    void SubmitStaticModel(const Ref<StaticModel>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitStaticModel(const RenderStaticSubmission& submission);

    void SubmitModel(const std::vector<std::string>& pls, const Ref<Model>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitModel(const std::vector<std::string>& pls, const RenderSubmission& submission);

    void SubmitStaticModel(const std::vector<std::string>& pls, const Ref<StaticModel>& model, const Ref<MaterialTable>& mat_table, const glm::mat4& transform, UUID material_id, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitStaticModel(const std::vector<std::string>& pls, const RenderStaticSubmission& submission);

    void SubmitDebugDrawCommands(const std::string_view pl, const std::vector<DebugDrawCommand>& cmds);

    // void DrawLine(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.f), float thickness = 1.f);
    // void DrawRect(const glm::vec3& min, const glm::vec3& max, const glm::vec4& color = glm::vec4(1.f));
    // void DrawTriangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec4& color = glm::vec4(1.f));

    bool Render();

    void Clear();

    Ref<Framebuffer> GetRender(UUID pipeline_id) const;

    enum RenderPassIndex {
      SHADOW_MAP = 0,
      DEPTH_PASS,
      GEOMETRY_PASS,

      NUM_RENDER_PASSES,
      INVALID_RENDER_PASSES = NUM_RENDER_PASSES
    };

    enum FramebufferIndex {
      SHADOW_MAP_FB = 0,
      DEPTH_TEXTURE_FB,
      FINAL_FRAME_FB,

      NUM_FRAMEBUFFERS,
      INVALID_FRAMEBUFFERS = NUM_FRAMEBUFFERS
    };

   private:
    struct FrameSubmissions {
      Ref<CameraBase> viewpoint = nullptr;
      Ref<LightEnvironment> environment = nullptr;
      Ref<MaterialTable> material_table = nullptr;

      Ref<UniformBuffer> camera_uniforms = nullptr;
      Ref<UniformBuffer> light_uniforms = nullptr;

      Ref<GBuffer> gbuffer = nullptr;

      ~FrameSubmissions();
    } frame_data;

    Ref<Framebuffer> framebuffers[NUM_FRAMEBUFFERS] = {
      nullptr,
      nullptr,
      nullptr,
    };

    Ref<RenderPass> render_passes[NUM_RENDER_PASSES] = {
      nullptr,
      nullptr,
      nullptr,
    };

    std::map<UUID, Ref<RenderPass>> custom_passes;
    std::map<UUID, Ref<Pipeline>> pipelines;
    std::map<UUID, Ref<Framebuffer>> image_ir;

    glm::ivec2 viewport_size;
    SceneRenderSpec spec;

    void Initialize();
    void Shutdown();

    void PreRenderSettings();

    bool FrameComplete() const;
    void ResetFrame();
  };

  struct PipelineRenderPassRegistration {
    std::vector<SceneRenderer::RenderPassIndex> passes;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCENE_RENDERER_HPp
