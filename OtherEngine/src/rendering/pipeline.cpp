/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

  RenderStaticSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
    };
  }

  RenderSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
    };
  }

  Pipeline::Pipeline(PipelineSpec& s)
      : spec(s) {
    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
    gbuffer = Ref<GBuffer>::Create(spec.framebuffer_spec.size);

    uint32_t model_binding_point = 1;
    std::vector<Uniform> model_uniforms = {
      { "models", ValueType::MAT4, 100 },

    };

    uint32_t material_binding_point = 2;
    std::vector<Uniform> material_uniforms = {
      { "materials", ValueType::USER_TYPE, 100, sizeof(Material) },
    };

    model_storage = NewRef<UniformBuffer>("ModelData", model_uniforms, model_binding_point, SHADER_STORAGE);
    material_storage = NewRef<UniformBuffer>("MaterialData", material_uniforms, material_binding_point, SHADER_STORAGE);
  }

  std::string Pipeline::Name() const {
    return spec.pipeline_name;
  }

  const FramebufferSpec& Pipeline::TargetSpec() const {
    return spec.framebuffer_spec;
  }

  const std::vector<Ref<RenderPass>>& Pipeline::GetRenderPasses() const {
    return passes;
  }

  void Pipeline::SetViewportSize(const glm::ivec2& size) {
    target->Resize(size);
  }

  void Pipeline::SubmitRenderPass(const Ref<RenderPass>& render_pass) {
    passes.push_back(render_pass);
  }

  void Pipeline::SubmitModel(const Ref<Model>& model, const glm::mat4& transform, const std::vector<Material>& materials, DrawMode topology) {
    SubmitModel({
      .model = model,
      .transform = transform,
      .materials = materials,
      .draw_mode = topology,
    });
  }

  void Pipeline::SubmitModel(const RenderSubmission& submission) {
    const std::vector<uint32_t>& sm_idxs = submission.model->SubMeshes();
    if (sm_idxs.empty()) {
      OE_ASSERT(submission.materials.size() == 1, "Model has no submeshes, but multiple materials submitted");
      SubmitStaticModel({
        .model = submission.model,
        .transform = submission.transform,
        .material = submission.materials[0],
        .draw_mode = submission.draw_mode,
      });
      return;
    }

    Ref<ModelSource> source = submission.model->GetModelSource();
    const std::vector<SubMesh>& submeshes = source->SubMeshes();

    for (const uint32_t sm_idx : sm_idxs) {
      OE_ASSERT(sm_idx < submeshes.size(), "Submesh index out of bounds");
      RenderStaticSubmission sub{
        .model = submission.model,
        .transform = submission.transform,
        .material = submission.materials[sm_idx],
        .draw_mode = submission.draw_mode,
      };

      MeshKey key = sub;
      key.submesh_idx = sm_idx;

      auto itr = model_submissions.find(key);
      if (itr == model_submissions.end()) {
        itr = InsertMeshKey(key, submission.model, sm_idx);
      }

      OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");
      auto& [mk, sl] = *itr;
      sl.cpu_model_storage.BufferData(submission.transform);
      sl.cpu_material_storage.BufferData(submission.materials[sm_idx]);
      ++sl.instance_count;
    }
  }

  void Pipeline::SubmitStaticModel(const Ref<StaticModel>& model, const glm::mat4& transform, const Material& material, DrawMode topology) {
    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material = material,
      .draw_mode = topology,
    });
  }

  void Pipeline::SubmitStaticModel(const RenderStaticSubmission& submission) {
    Ref<ModelSource> source = submission.model->GetModelSource();
    MeshKey key = submission;
    key.submesh_idx = 0;

    auto itr = model_submissions.find(key);
    if (itr == model_submissions.end()) {
      itr = InsertStaticMeshKey(key, submission.model);
    }

    OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");
    auto& [mk, sl] = *itr;
    sl.cpu_model_storage.BufferData(submission.transform);
    sl.cpu_material_storage.BufferData(submission.material);
    ++sl.instance_count;
  }

  void Pipeline::Render(bool render_gbuffer) {
    material_storage->Clear();
    model_storage->Clear();

    if (render_gbuffer) {
      gbuffer->Bind();
      CHECKGL();

      RenderAll();
      CHECKGL();

      gbuffer->Unbind();
      CHECKGL();
    }

    target->BindFrame();
    CHECKGL();
    for (auto& pass : passes) {
      if (pass == nullptr) {
        continue;
      }

      PerformPass(pass);
      CHECKGL();
    }
    target->UnbindFrame();
    CHECKGL();
  }

  Ref<Framebuffer> Pipeline::GetOutput() const {
    return target;
  }

  GBuffer& Pipeline::GetGBuffer() {
    OE_ASSERT(gbuffer != nullptr, "GBuffer is null!");
    return *gbuffer;
  }

  void Pipeline::Clear() {
    /// dont clear the mesh key for a tiny optimization on future submissions
    for (auto& [mk, sl] : model_submissions) {
      sl.cpu_model_storage.ZeroMem();
      sl.cpu_material_storage.ZeroMem();
      sl.instance_count = 0;
    }
    model_submissions.clear();
  }

  void Pipeline::PerformPass(Ref<RenderPass>& pass) {
    CHECKGL();

    pass->Bind();
    pass->SetInput("goe_position", 0);
    pass->SetInput("goe_normal", 1);
    pass->SetInput("goe_albedo", 2);
    pass->SetInput("goe_specular", 3);
    pass->SetInput("goe_shadow_map", 4);
    pass->SetInput("goe_depth_map", 5);

    CHECKGL();

    RenderAll();

    CHECKGL();

    pass->Unbind();
    CHECKGL();
  }

  FrameMeshes::iterator Pipeline::InsertMeshKey(MeshKey& key, const Ref<Model>& model, uint32_t submesh_idx) {
    OE_ASSERT(model != nullptr, "Model is null");
    OE_ASSERT(model->GetModelSource() != nullptr, "Model source is null");
    OE_ASSERT(submesh_idx < model->SubMeshes().size(), "Submesh index out of bounds");

    Ref<VertexArray> vao = Ref<VertexArray>::Clone(model->model_vaos[submesh_idx]);
    OE_ASSERT(vao != nullptr, "Failed to clone vertex array");

    MeshSubmissionList msl{
      .vao = vao,
      .num_elements = vao->NumElements(),
      .instance_count = 0,
      .cpu_model_storage = Buffer(),
      .cpu_material_storage = Buffer(),
    };
    msl.base_vertex = 0;
    msl.base_instance = 0;

    return model_submissions.insert({ key, std::move(msl) }).first;
  }

  FrameMeshes::iterator Pipeline::InsertStaticMeshKey(MeshKey& key, const Ref<StaticModel>& model) {
    OE_ASSERT(model != nullptr, "Static model is null");
    OE_ASSERT(model->GetModelSource() != nullptr, "Static model source is null");

    Ref<VertexArray> vao = Ref<VertexArray>::Clone(model->model_vao);
    MeshSubmissionList msl{
      .vao = vao,
      .num_elements = vao->NumElements(),
      .instance_count = 0,
      .cpu_model_storage = Buffer(),
      .cpu_material_storage = Buffer(),
    };

    msl.base_vertex = 0;
    msl.base_instance = 0;

    return model_submissions.insert({ key, std::move(msl) }).first;
  }

  void Pipeline::RenderAll() {
    for (auto& [mk, sl] : model_submissions) {
      RenderMeshes(mk, sl);
    }
  }

  void Pipeline::RenderMeshes(const MeshKey& mesh_key, MeshSubmissionList& msl) {
    model_storage->BindBase();
    CHECKGL();
    model_storage->LoadFromBuffer(msl.cpu_model_storage);
    CHECKGL();

    material_storage->BindBase();
    material_storage->LoadFromBuffer(msl.cpu_material_storage);

    msl.vao->Bind();
    glPolygonMode(GL_FRONT_AND_BACK, mesh_key.render_state);
    glDrawElementsInstancedBaseVertexBaseInstance(mesh_key.draw_mode, msl.num_elements, GL_UNSIGNED_INT, (void*)0, msl.instance_count, msl.base_vertex, msl.base_instance);
    CHECKGL();

    msl.vao->Unbind();
  }

}  // namespace other
