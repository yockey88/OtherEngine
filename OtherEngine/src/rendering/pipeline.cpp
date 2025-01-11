/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/logger.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/draw_calls.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

  Pipeline::Pipeline(PipelineSpec& s)
      : spec(s) {
    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);

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

  void Pipeline::SubmitModel(const Ref<Model>& model, const Ref<MaterialTable>& material_table, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    SubmitModel({
      .model = model,
      .transform = transform,
      .material_table = material_table,
      .material = material_id,
      .draw_mode = topology,
    });
  }

  void Pipeline::SubmitModel(const RenderSubmission& submission) {
    OE_ASSERT(submission.model != nullptr, "Model is null");

    Ref<ModelSource> source = submission.model->GetModelSource();
    OE_ASSERT(source != nullptr, "Model source is null");

    const std::vector<SubMesh>& submeshes = source->SubMeshes();
    OE_ASSERT(!submeshes.empty(), "Model source has no submeshes");

    const std::vector<uint32_t>& sm_idxs = submission.model->SubMeshes();
    OE_ASSERT(!sm_idxs.empty(), "Model has no submeshes");

    MeshKey key = submission;

    auto itr = model_submissions.find(key);
    if (itr == model_submissions.end()) {
      itr = InsertMeshKey(key, submission.model);
    }
    OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");

    auto& [mk, sl] = *itr;

    Ref<MaterialTable> material_table = submission.material_table;

    OE_ASSERT(material_table != nullptr, "Material table is null");

    sl.submissions.resize(submeshes.size());
    for (uint32_t i = 0; i < sm_idxs.size(); ++i) {
      const uint32_t sm_idx = sm_idxs[i];
      OE_ASSERT(sm_idx < submeshes.size(), "Submesh index out of bounds");

      const SubMesh& sub_mesh = submeshes[sm_idx];
      SubMeshDrawCall& smdc = sl.submissions[sm_idx];

      UUID material_id = sub_mesh.material_id.Get() == 0 ? material_table->DefaultMaterial() : sub_mesh.material_id;
      OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
      Material gpumat = material_table->GetMaterial(material_id);

      smdc.cpu_model_storage.BufferData(submission.transform);
      smdc.cpu_material_storage.BufferData(gpumat);
      smdc.instance_count++;
    }
  }

  void Pipeline::SubmitStaticModel(const Ref<StaticModel>& model, const Ref<MaterialTable>& material_table, const glm::mat4& transform, UUID material_id, DrawMode topology) {
    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material_table = material_table,
      .material = material_id,
      .draw_mode = topology,
    });
  }

  void Pipeline::SubmitStaticModel(const RenderStaticSubmission& submission) {
    Ref<ModelSource> source = submission.model->GetModelSource();
    OE_ASSERT(source != nullptr, "Model source is null");

    MeshKey key = submission;

    auto itr = static_model_submissions.find(key);
    if (itr == static_model_submissions.end()) {
      itr = InsertStaticMeshKey(key, submission.model);
    }

    OE_ASSERT(itr != static_model_submissions.end(), "Failed to insert mesh key");
    auto& [mk, sl] = *itr;
    OE_ASSERT(sl.vao != nullptr, "Mesh submission list has null vertex array");

    Ref<MaterialTable> material_table = submission.material_table;
    OE_ASSERT(material_table != nullptr, "Material table is null");

    UUID material_id = submission.material.Get() == 0 ? material_table->DefaultMaterial() : submission.material;
    OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
    Material gpumat = material_table->GetMaterial(material_id);

    sl.cpu_model_storage.BufferData(submission.transform);
    sl.cpu_material_storage.BufferData(gpumat);
    sl.index_count = sl.vao->NumElements();
    ++sl.instance_count;
  }

  void Pipeline::SubmitDebugDrawCommands(const std::vector<DebugDrawCommand>& cmds) {
    debug_draw_commands.insert(debug_draw_commands.end(), cmds.begin(), cmds.end());
  }

  void Pipeline::Render() {
    material_storage->Clear();
    model_storage->Clear();

    target->BindFrame();
    CHECKGL();
    for (auto& pass : passes) {
      if (pass == nullptr) {
        continue;
      }

      PerformPass(pass);
      CHECKGL();
    }

    /// HACK:
    for (const DebugDrawCommand& cmd : debug_draw_commands) {
      cmd();
    }

    target->UnbindFrame();
    CHECKGL();
  }

  Ref<Framebuffer> Pipeline::GetOutput() const {
    return target;
  }

  void Pipeline::Clear() {
    /// dont clear the mesh key for a tiny optimization on future submissions
    for (auto& [mk, sl] : static_model_submissions) {
      sl.cpu_model_storage.ZeroMem();
      sl.cpu_material_storage.ZeroMem();
      sl.instance_count = 0;
    }
    for (auto& [mk, sl] : model_submissions) {
      for (auto& sub : sl.submissions) {
        sub.cpu_model_storage.ZeroMem();
        sub.cpu_material_storage.ZeroMem();
        sub.instance_count = 0;
      }
    }
    static_model_submissions.clear();
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

  FrameMeshes::iterator Pipeline::InsertMeshKey(MeshKey& key, const Ref<Model>& model) {
    OE_ASSERT(model != nullptr, "Model is null");

    const std::vector<uint32_t>& sm_idxs = model->SubMeshes();
    OE_ASSERT(!sm_idxs.empty(), "Model has no submeshes");

    Ref<ModelSource> source = model->GetModelSource();
    OE_ASSERT(source != nullptr, "Model source is null");

    const std::vector<SubMesh>& submeshes = source->SubMeshes();
    OE_ASSERT(!submeshes.empty(), "Model source has no submeshes");

    Ref<VertexArray> vao = Ref<VertexArray>::Clone(source->source_vao);

    MeshDrawCall msl = {
      .vao = vao,
      .base_instance = 0,
      .submissions = {},
      .line_thickness = key.line_thickness,
    };
    msl.submissions.reserve(sm_idxs.size());
    for (auto& sm_idx : sm_idxs) {
      const SubMesh& sub_mesh = submeshes[sm_idx];
      msl.submissions.push_back({
        .cpu_model_storage = Buffer(),
        .cpu_material_storage = Buffer(),
        .vertex_offset = sub_mesh.base_vertex,
        .vertex_count = sub_mesh.vert_cnt,
        .index_offset = sub_mesh.base_idx,
        .index_count = sub_mesh.idx_cnt,
      });
    }

    return model_submissions.insert({ key, std::move(msl) }).first;
  }

  StaticFrameMeshes::iterator Pipeline::InsertStaticMeshKey(MeshKey& key, const Ref<StaticModel>& model) {
    OE_ASSERT(model != nullptr, "Static model is null");
    OE_ASSERT(model->GetModelSource() != nullptr, "Static model source is null");

    Ref<VertexArray> vao = Ref<VertexArray>::Clone(model->model_vao);
    OE_ASSERT(vao != nullptr, "Failed to clone vertex array");

    StaticMeshDrawCall msl = {
      .vao = vao,
      .cpu_model_storage = Buffer(),
      .cpu_material_storage = Buffer(),
      .instance_count = 0,
      .index_count = 0,
      .line_thickness = key.line_thickness,
    };

    return static_model_submissions.insert({ key, std::move(msl) }).first;
  }

  void Pipeline::SubmitDrawCall(const MeshKey& key, const DrawCall& call) {
    OE_ASSERT(call.vao != nullptr, "Mesh submission list has null vertex array");
    // auto itr = draw_calls.find(key);
    // if (itr == draw_calls.end()) {
    //   Ref<VertexArray> vao = Ref<VertexArray>::Clone(source->source_vao);
    //   OE_ASSERT(vao != nullptr, "Failed to clone vertex array");

    //   DrawCall msl = {
    //     .vao = vao,
    //     .cpu_model_storage = Buffer(),
    //     .cpu_material_storage = Buffer(),
    //     .instance_count = 0,
    //     .vertex_offset = 0,
    //     .index_offset = 0,
    //     .index_count = vao->NumElements(),
    //   };

    //   auto [itr, res] = draw_calls.insert({ key, std::move(msl) });
    //   OE_ASSERT(res, "Failed to insert draw call");
    // }

    // itr = draw_calls.find(key);
    // OE_ASSERT(itr != draw_calls.end(), "Failed to find draw call");

    // auto& [mk, sl] = *itr;
    // OE_ASSERT(sl.vao != nullptr, "Mesh submission list has null vertex array");

    // Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    // OE_ASSERT(material_table != nullptr, "Material table is null");

    // UUID material_id = submission.material.Get() == 0 ? material_table->DefaultMaterial() : submission.material;
    // OE_ASSERT(material_table->HasMaterial(material_id), "Material not found in table");
    // Material gpumat = material_table->GetMaterial(material_id);

    // sl.cpu_model_storage.BufferData(submission.transform);
    // sl.cpu_material_storage.BufferData(gpumat);
    // sl.instance_count++;
  }

  void Pipeline::RenderAll() {
    for (auto& [mk, draw_call] : static_model_submissions) {
      RenderStaticMeshes(mk, draw_call);
    }
    for (auto& [mk, draw_call] : model_submissions) {
      RenderMeshes(mk, draw_call);
    }
  }

  void Pipeline::RenderStaticMeshes(const MeshKey& mesh_key, StaticMeshDrawCall& draw_call) {
    OE_ASSERT(material_storage != nullptr, "Material storage is null");
    OE_ASSERT(model_storage != nullptr, "Model storage is null");
    OE_ASSERT(draw_call.vao != nullptr, "Mesh submission list has null vertex array");

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    draw_call.vao->Bind();
    material_table->Bind();
    CHECKGL();

    model_storage->BindBase();
    model_storage->LoadFromBuffer(draw_call.cpu_model_storage);
    CHECKGL();

    material_storage->BindBase();
    material_storage->LoadFromBuffer(draw_call.cpu_material_storage);

    glLineWidth(draw_call.line_thickness);
    glPolygonMode(GL_FRONT_AND_BACK, mesh_key.render_state);
    glDrawElementsInstancedBaseVertexBaseInstance(mesh_key.draw_mode, draw_call.index_count, GL_UNSIGNED_INT, (void*)0, draw_call.instance_count, 0, 0);

    material_table->Unbind();
    draw_call.vao->Unbind();
  }

  /**
   * @note this is slooooow, (also broken at the moment....)
   *        Goals:
   *         - bindless textures to avoid filling the material storage for each submesh
   *         - Giant global vertex buffer with offsets for each model and offsets for each model's submesh
   *              (maybe runtime option? when export final game/select in menu bake mesh into the runtime asset binary)
   */
  void Pipeline::RenderMeshes(const MeshKey& mesh_key, MeshDrawCall& draw_call) {
    OE_ASSERT(material_storage != nullptr, "Material storage is null");
    OE_ASSERT(model_storage != nullptr, "Model storage is null");
    OE_ASSERT(draw_call.vao != nullptr, "Mesh submission list has null vertex array");

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    draw_call.vao->Bind();
    material_table->Bind();
    CHECKGL();

    for (auto& sub_call : draw_call.submissions) {
      model_storage->BindBase();
      model_storage->LoadFromBuffer(sub_call.cpu_model_storage);
      material_storage->BindBase();
      material_storage->LoadFromBuffer(sub_call.cpu_material_storage);

      glLineWidth(draw_call.line_thickness);
      glPolygonMode(GL_FRONT_AND_BACK, mesh_key.render_state);
      glDrawElementsInstancedBaseVertexBaseInstance(mesh_key.draw_mode, sub_call.index_count, GL_UNSIGNED_INT, (void*)0, sub_call.instance_count, sub_call.vertex_offset, 0);
      CHECKGL();
    }

    material_table->Unbind();
    draw_call.vao->Unbind();
  }

}  // namespace other
