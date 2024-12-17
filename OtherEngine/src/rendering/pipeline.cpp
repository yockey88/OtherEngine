/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "asset/asset_manager.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

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

  void Pipeline::SubmitModel(const Ref<Model>& model, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    SubmitModel({
      .model = model,
      .transform = transform,
      .material = material_id,
      .draw_mode = topology,
    });
  }

  void Pipeline::SubmitModel(const RenderSubmission& submission) {
    const std::vector<uint32_t>& sm_idxs = submission.model->SubMeshes();

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    if (sm_idxs.empty()) {
      MeshKey key = submission;
      key.submesh_idx = 0;

      auto itr = model_submissions.find(key);
      if (itr == model_submissions.end()) {
        itr = InsertMeshKey(key, submission.model, 0);
      }

      OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");
      auto& [mk, sl] = *itr;

      // UUID material_id = submission.material.Get() == 0 ? material_table->DefaultMaterial() : submission.material;
      UUID material_id = material_table->DefaultMaterial();

      OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
      Material gpumat = material_table->GetMaterial(material_id);

      sl.cpu_model_storage.BufferData(submission.transform);
      sl.cpu_material_storage.BufferData(gpumat);
      ++sl.instance_count;
      return;
    }
    OE_ASSERT(false, "Multi-submesh rendering not implemented");

    Ref<ModelSource> source = submission.model->GetModelSource();
    OE_ASSERT(source != nullptr, "Model source is null");

    const std::vector<SubMesh>& submeshes = source->SubMeshes();

    for (const uint32_t sm_idx : sm_idxs) {
      OE_ASSERT(sm_idx < submeshes.size(), "Submesh index out of bounds");
      RenderSubmission sub{
        .model = submission.model,
        .transform = submission.transform,
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

      // UUID material_id = submission.material.Get() == 0 ? material_table->DefaultMaterial() : submission.material;
      UUID material_id = material_table->DefaultMaterial();

      OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
      Material gpumat = material_table->GetMaterial(material_id);

      sl.cpu_model_storage.BufferData(submission.transform);
      sl.cpu_material_storage.BufferData(gpumat);
      ++sl.instance_count;
    }
  }

  void Pipeline::SubmitStaticModel(const Ref<StaticModel>& model, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material = material_id,
      .draw_mode = topology,
    });
  }

  void Pipeline::SubmitStaticModel(const RenderStaticSubmission& submission) {
    Ref<ModelSource> source = submission.model->GetModelSource();
    OE_ASSERT(source != nullptr, "Model source is null");

    MeshKey key = submission;
    key.submesh_idx = 0;

    auto itr = model_submissions.find(key);
    if (itr == model_submissions.end()) {
      itr = InsertStaticMeshKey(key, submission.model);
    }

    OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");
    auto& [mk, sl] = *itr;

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    // UUID material_id = submission.material.Get() == 0 ? material_table->DefaultMaterial() : submission.material;
    UUID material_id = material_table->DefaultMaterial();

    OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
    Material gpumat = material_table->GetMaterial(material_id);

    sl.cpu_model_storage.BufferData(submission.transform);
    sl.cpu_material_storage.BufferData(gpumat);
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
    // pass->SetInput("goe_position", 0);
    // pass->SetInput("goe_normal", 1);
    // pass->SetInput("goe_albedo", 2);
    // pass->SetInput("goe_specular", 3);
    pass->SetInput("albedo_textures", 0);
    pass->SetInput("normal_textures", 1);
    pass->SetInput("roughness_textures", 2);
    pass->SetInput("goe_shadow_map", 3);
    pass->SetInput("goe_depth_map", 4);

    CHECKGL();

    RenderAll();

    CHECKGL();

    pass->Unbind();
    CHECKGL();
  }

  FrameMeshes::iterator Pipeline::InsertMeshKey(MeshKey& key, const Ref<Model>& model, uint32_t submesh_idx) {
    OE_ASSERT(model != nullptr, "Model is null");
    OE_ASSERT(model->GetModelSource() != nullptr, "Model source is null");
    if (submesh_idx == 0) {
      OE_ASSERT(model->SubMeshes().empty(), "Model has submeshes but submesh index is 0");
    } else if (!model->SubMeshes().empty()) {
      OE_ASSERT(submesh_idx < model->SubMeshes().size(), "Submesh index out of bounds");
    }

    Ref<VertexArray> vao = Ref<VertexArray>::Clone(model->model_vaos[0]);
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

  void Pipeline::RenderAll() {
    for (auto& [mk, sl] : model_submissions) {
      RenderMeshes(mk, sl);
    }
  }

  void Pipeline::RenderMeshes(const MeshKey& mesh_key, MeshSubmissionList& msl) {
    OE_ASSERT(msl.vao != nullptr, "Mesh submission list has null vertex array");

    model_storage->BindBase();
    CHECKGL();
    model_storage->LoadFromBuffer(msl.cpu_model_storage);
    CHECKGL();

    material_storage->BindBase();
    material_storage->LoadFromBuffer(msl.cpu_material_storage);

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    msl.vao->Bind();
    CHECKGL();

    material_table->Bind();
    CHECKGL();

    /// gbuffer takes 0-3, shadow map and depth are 4,5 so we start at 6
    /// FIXME: make this more dynamic
    glPolygonMode(GL_FRONT_AND_BACK, mesh_key.render_state);
    glDrawElementsInstancedBaseVertexBaseInstance(mesh_key.draw_mode, msl.num_elements, GL_UNSIGNED_INT, (void*)0, msl.instance_count, msl.base_vertex, msl.base_instance);
    CHECKGL();

    msl.vao->Unbind();
  }

}  // namespace other
