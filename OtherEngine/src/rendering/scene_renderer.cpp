/**
 * \file rendering/scene_renderer.cpp
 **/
#include "rendering/scene_renderer.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "rendering/geometry_pass.hpp"
#include "rendering/pipeline.hpp"
#include "rendering/uniform.hpp"

namespace other {

  SceneRenderer::SceneRenderer(SceneRenderSpec spec)
      : spec(spec) {
    Initialize();
  }

  SceneRenderer::~SceneRenderer() {
  }

  void SceneRenderer::SetViewportSize(const glm::ivec2& size) {
  }

  void SceneRenderer::SubmitCamera(Ref<CameraBase>& camera) {
    if (frame_data.viewpoint != nullptr) {
      /// only one viewpoint per frame
      return;
    }
    OE_ASSERT(camera != nullptr, "Camera is null");
    OE_ASSERT(camera_uniforms != nullptr, "Camera uniforms are null");

    const glm::mat4& proj = camera->ProjectionMatrix();
    const glm::mat4& view = camera->ViewMatrix();
    glm::vec4 cam_pos = glm::vec4(camera->Position(), 1.f);

    camera_uniforms->SetUniform("projection", proj);
    camera_uniforms->SetUniform("view", view);
    camera_uniforms->SetUniform("viewpoint", cam_pos);
    frame_data.viewpoint = camera;
  }

  void SceneRenderer::SubmitEnvironment(Ref<LightEnvironment>& environment) {
    if (frame_data.environment != nullptr) {
      /// only one environment per frame
      return;
    }
    OE_ASSERT(environment != nullptr, "Environment is null");
    OE_ASSERT(light_uniforms != nullptr, "Light uniforms are null");

    float num_dir_lights = environment->direction_lights.size();
    float num_point_lights = environment->point_lights.size();
    glm::vec4 light_count{
      num_dir_lights, num_point_lights,
      0, 0
    };
    light_uniforms->BindBase();
    light_uniforms->SetUniform("num_lights", light_count);
    for (size_t i = 0; i < num_point_lights; ++i) {
      auto& l = environment->point_lights[i];
      light_uniforms->SetUniform("point_lights", l, i);
    }
    for (size_t i = 0; i < num_dir_lights; ++i) {
      auto& l = environment->direction_lights[i];
      light_uniforms->SetUniform("direction_lights", l, i);
    }
    frame_data.environment = environment;
  }

  void SceneRenderer::ClearLightEnvironment() {
    glm::vec4 light_count{ 0, 0, 0, 0 };
    light_uniforms->BindBase();
    light_uniforms->SetUniform("num_lights", light_count);
  }

  void SceneRenderer::SubmitModel(Ref<Model> model, const glm::mat4& transform, const Material& material, DrawMode topology) {
    if (model == nullptr) {
      return;
    }

    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material = material,
      .draw_mode = topology,
    });

    // for (auto& [id, pl] : pipelines) {
    //   pl->SubmitModel(model, transform, material);
    // }
  }

  void SceneRenderer::SubmitStaticModel(Ref<StaticModel> model, const glm::mat4& transform, const Material& material, DrawMode topology) {
    if (model == nullptr) {
      return;
    }

    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material = material,
      .draw_mode = topology,
    });

    // for (auto& [id, pl] : pipelines) {
    //   pl->SubmitStaticModel(model, transform, material);
    // }
  }

  void SceneRenderer::SubmitStaticModel(const RenderSubmission& submission) {
    if (submission.model == nullptr) {
      return;
    }
    Ref<ModelSource> source = submission.model->GetModelSource();
    MeshKey key = submission;

    auto itr = model_submissions.find(key);
    if (itr == model_submissions.end()) {
      auto& verts = submission.model->GetModelSource()->RawVertices();
      auto& idxs = submission.model->GetModelSource()->Indices();

      itr = InsertMeshKey(key, verts, idxs);
    }

    OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");
    auto& [mk, sl] = *itr;
    sl.cpu_model_storage.BufferData(submission.transform);
    sl.cpu_material_storage.BufferData(submission.material);
    ++sl.instance_count;

    // for (auto& [id, pl] : pipelines) {
    //   pl->SubmitStaticModel(submission);
    // }
  }

  void SceneRenderer::RenderGbuffer() {
    OE_ASSERT(gbuffer != nullptr, "GBuffer is null!");
    OE_ASSERT(model_storage != nullptr, "Model storage is null!");
    OE_ASSERT(material_storage != nullptr, "Material storage is null!");
    if (!FrameComplete()) {
      return;
    }

    PreRenderSettings();

    material_storage->Clear();
    model_storage->Clear();

    gbuffer->Bind();
    CHECKGL();

    for (auto& [mk, sl] : model_submissions) {
      RenderMeshList(mk, sl.instance_count, sl.cpu_model_storage, sl.cpu_material_storage);
    }
    CHECKGL();

    gbuffer->Unbind();
    CHECKGL();
  }

  bool SceneRenderer::RenderAll() {
    if (!FrameComplete()) {
      return false;
    }

    PreRenderSettings();
    RenderGbuffer();
    Render();
    return true;
  }

  bool SceneRenderer::FinalizeScene() {
    if (!RenderAll()) {
      return false;
    }
    ResetFrame();
    return true;
  }

  void SceneRenderer::Clear() {
    /// dont clear the mesh key for a tiny optimization on future submissions
    for (auto& [mk, sl] : model_submissions) {
      sl.cpu_model_storage.ZeroMem();
      sl.cpu_material_storage.ZeroMem();
      sl.instance_count = 0;
    }
    model_submissions.clear();
  }

  const Ref<Framebuffer>& SceneRenderer::GetRender() const {
    return framebuffers.final_frame;
  }

  void SceneRenderer::Initialize() {
    Path engine_core_dir = Filesystem::GetEngineCoreDir();
    Path geometry_shader_path = engine_core_dir / "OtherEngine" / "assets" / "shaders" / "default.oshader";

    Ref<Directory> shader_dir = Filesystem::GetDirectory("shaders");
    Ref<FileHandle> depth_shader_file = shader_dir->GetFile("depth_shader.oshader");
    OE_ASSERT(depth_shader_file != nullptr, "Failed to get shader file : {}", "depth_shader.oshader");

    Ref<Shader> depth_shader = BuildShader(depth_shader_file->AbsolutePath());
    RenderPassSpec depth_spec = {
      .name = "depth-pass",
      .tag_col = { 1.f, 0.f, 0.f, 1.f },
      .shader = depth_shader,
    };

    std::vector<Uniform> geometry_unis = {};
    Ref<Shader> geometry_shader = BuildShader(geometry_shader_path);

    render_passes.depth_pass = NewRef<RenderPass>(depth_spec);
    render_passes.geometry_pass = NewRef<GeometryPass>(geometry_unis, geometry_shader);

    uint32_t camera_binding_pnt = spec.camera_binding_pnt;
    std::vector<Uniform> cam_unis = spec.cam_unis.size() > 0 ?
      spec.cam_unis :
      std::vector<Uniform>{
        { "projection", ValueType::MAT4 },
        { "view", ValueType::MAT4 },
        { "viewpoint", ValueType::VEC4 },
      };

    uint32_t light_binding_pnt = spec.light_binding_pnt;
    std::vector<Uniform> light_unis = spec.light_unis.size() > 0 ?
      spec.light_unis :
      std::vector<Uniform>{
        { "num_lights", ValueType::VEC4 },
        { "point_lights", ValueType::USER_TYPE, 100, sizeof(PointLight) },
        { "direction_lights", ValueType::USER_TYPE, 100, sizeof(DirectionLight) },
      };

    spec.vertex_layout = {
      { ValueType::VEC3, "position" },
      { ValueType::VEC3, "normal" },
      { ValueType::VEC3, "tangent" },
      { ValueType::VEC3, "binormal" },
      { ValueType::VEC2, "uvs" }
    };

    spec.model_binding_point = 1;
    spec.model_uniforms = {
      { "models", ValueType::MAT4, 100 },
    };
    spec.material_binding_point = 2;
    spec.material_uniforms = {
      { "materials", ValueType::USER_TYPE, 100, sizeof(Material) },
    };

    gbuffer = NewRef<GBuffer>(glm::ivec2{ 1920, 1080 });
    camera_uniforms = NewRef<UniformBuffer>("Camera", cam_unis, camera_binding_pnt);
    model_storage = NewRef<UniformBuffer>("ModelData", spec.model_uniforms, spec.model_binding_point, SHADER_STORAGE);
    material_storage = NewRef<UniformBuffer>("MaterialData", spec.material_uniforms, spec.material_binding_point, SHADER_STORAGE);
    light_uniforms = NewRef<UniformBuffer>("Lights", light_unis, light_binding_pnt, SHADER_STORAGE);

    OE_ASSERT(camera_uniforms != nullptr, "Failed to create camera uniforms");
    OE_ASSERT(light_uniforms != nullptr, "Failed to create light uniforms");

    camera_uniforms->BindBase();
    model_storage->BindBase();
    material_storage->BindBase();
    light_uniforms->BindBase();

    FramebufferSpec shadow_map_spec = {
      .depth_func = LESS,
      .clear_color = { 0.f, 0.5f, 0.4f, 1.f },
      .size = { 1920, 1080 },
      .depth = true,
      .color = false,
      .stencil = false,
    };
    framebuffers.shadow_map = NewRef<Framebuffer>(shadow_map_spec);

    FramebufferSpec depth_tex_spec = {
      .depth_func = LESS,
      .clear_color = { 0.f, 0.f, 0.f, 1.f },
      .size = { 1920, 1080 },
    };
    framebuffers.depth_texture = NewRef<Framebuffer>(depth_tex_spec);

    FramebufferSpec final_frame_spec = {
      .depth_func = LESS_EQUAL,
      .clear_color = { 0.1f, 0.1f, 0.1f, 1.f },
      .size = { 1920, 1080 },
    };
    framebuffers.final_frame = NewRef<Framebuffer>(final_frame_spec);
  }

  void SceneRenderer::Shutdown() {
    gbuffer = nullptr;

    framebuffers.shadow_map = nullptr;
    framebuffers.depth_texture = nullptr;
    framebuffers.final_frame = nullptr;

    model_submissions.clear();

    camera_uniforms = nullptr;
    light_uniforms = nullptr;
    model_storage = nullptr;
    material_storage = nullptr;
  }

  void SceneRenderer::PreRenderSettings() {
    /// TODO: figure out why glPolygonMode causes INVALID ENUM ?? here
    ///       but not below
  }

  void SceneRenderer::Render() {
    material_storage->Clear();
    model_storage->Clear();

    /** Passes to implement
     * ----------------
     * shadow mapping (expensive) :
     *  shadow map pass
     *  spot shadow map pass
     *
     * pre depth pass
     * hzb compute
     * pre integration
     * light culling
     * skybox pass
     * geometry pass
     *
     * if GTAO passes:
     *  GTAO compute
     *  GTAO denoise compute
     *  AO Composite
     *
     * pre convolution compute
     *
     * if jump flood:
     *  jump flood
     *
     * if SSR pases:
     *  ssr compute
     *  ssr composite
     *
     * if edge detection passes:
     *  edge detection
     *
     * bloom compute
     * composite pass
     **/

    framebuffers.shadow_map->BindFrame();
    PerformPass(render_passes.shadow_map_pass);
    framebuffers.shadow_map->UnbindFrame();

    framebuffers.depth_texture->BindFrame();
    PerformPass(render_passes.depth_pass);
    framebuffers.depth_texture->UnbindFrame();
    CHECKGL();

    framebuffers.final_frame->BindFrame();
    PerformPass(render_passes.geometry_pass);
    framebuffers.final_frame->UnbindFrame();
    CHECKGL();
  }

  void SceneRenderer::PerformDepthPass() {
  }

  void SceneRenderer::PerformPass(Ref<RenderPass>& pass) {
    OE_ASSERT(pass != nullptr, "Render pass is null!");
    CHECKGL();

    pass->Bind();
    pass->SetInput("goe_position", 0);
    pass->SetInput("goe_normal", 1);
    pass->SetInput("goe_albedo", 2);

    CHECKGL();
    for (auto& [mk, sl] : model_submissions) {
      RenderMeshList(mk, sl.instance_count, sl.cpu_model_storage, sl.cpu_material_storage);
    }

    CHECKGL();

    pass->Unbind();
    CHECKGL();
  }

  void SceneRenderer::RenderMeshList(const MeshKey& mesh_key, uint32_t instance_count, const Buffer& model_buffer, const Buffer& material_buffer) {
    model_storage->BindBase();
    CHECKGL();
    model_storage->LoadFromBuffer(model_buffer);
    CHECKGL();

    material_storage->BindBase();
    material_storage->LoadFromBuffer(material_buffer);

    mesh_key.vao->Bind();

    glPolygonMode(GL_FRONT_AND_BACK, mesh_key.render_state);
    glDrawElementsInstancedBaseVertexBaseInstance(mesh_key.draw_mode, mesh_key.num_elements, GL_UNSIGNED_INT, (void*)0, instance_count, 0, 0);
    CHECKGL();
  }

  bool SceneRenderer::FrameComplete() const {
    return frame_data.viewpoint != nullptr &&
      frame_data.environment != nullptr;
  }

  void SceneRenderer::ResetFrame() {
    frame_data.viewpoint = nullptr;
    frame_data.environment = nullptr;
  }

  FrameMeshes::iterator SceneRenderer::InsertMeshKey(MeshKey& key, const std::vector<float>& vertices, const std::vector<Index>& indices) {
    std::vector<uint32_t> idxs{};
    for (const auto& i : indices) {
      idxs.push_back(i.v1);
      idxs.push_back(i.v2);
      idxs.push_back(i.v3);
    }

    key.vao = NewRef<VertexArray>(vertices, idxs);
    key.num_elements = key.vao->NumElements();

    MeshSubmissionList msl{
      .instance_count = 0,
      .cpu_model_storage = Buffer(),  /// pre-allocate here?
      .cpu_material_storage = Buffer(),
    };

    return model_submissions.insert({ key, std::move(msl) }).first;
  }

}  // namespace other
