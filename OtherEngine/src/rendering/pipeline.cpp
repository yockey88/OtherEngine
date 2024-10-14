/**
 * \file rendering/pipeline.cpp
 **/
#include "rendering/pipeline.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

struct MeshKeyComparison {
  bool operator()(const other::MeshKey& lhs, const other::MeshKey& rhs) const {
    if (lhs.source_handle.Get() != rhs.source_handle.Get()) {
      return lhs.source_handle.Get() < rhs.source_handle.Get();
    }

    if (lhs.render_state != rhs.render_state) {
      return lhs.render_state < rhs.render_state;
    }

    if (lhs.draw_mode != rhs.draw_mode) {
      return lhs.draw_mode < rhs.draw_mode;
    }

    return lhs.selected && !rhs.selected;
  }
};
constexpr inline MeshKeyComparison mesh_key_compare{};

namespace other {

  auto MeshKey::operator<=>(const MeshKey& other) const {
    if (mesh_key_compare(*this, other)) {
      return -1;
    } else if (mesh_key_compare(other, *this)) {
      return 1;
    } else {
      return 0;
    }
  }

  RenderSubmission::operator MeshKey() const {
    return {
      .source_handle = model->GetModelSource()->handle,
      .render_state = render_state,
      .draw_mode = draw_mode,
    };
  }

  Pipeline::Pipeline(PipelineSpec& s)
      : spec(s), gbuffer(s.framebuffer_spec.size) {
    target = Ref<Framebuffer>::Create(spec.framebuffer_spec);
    model_storage = NewRef<UniformBuffer>("ModelData", spec.model_uniforms, spec.model_binding_point, SHADER_STORAGE);
    material_storage = NewRef<UniformBuffer>("MaterialData", spec.material_uniforms, spec.material_binding_point, SHADER_STORAGE);
  }

  void Pipeline::SubmitRenderPass(const Ref<RenderPass>& render_pass) {
    passes.push_back(render_pass);
  }

  void Pipeline::SubmitModel(Ref<Model> model, const glm::mat4& transform, const Material& material) {
  }

  void Pipeline::SubmitStaticModel(Ref<StaticModel> model, const glm::mat4& transform, const Material& material) {
    SubmitStaticModel({
      .model = model,
      .transform = transform,
      .material = material,
      .draw_mode = spec.topology,
    });
  }

  void Pipeline::SubmitStaticModel(const RenderSubmission& submission) {
    Ref<ModelSource> source = submission.model->GetModelSource();
    MeshKey key = submission;

    auto itr = std::ranges::find_if(model_submissions, [&](const auto& pair) -> bool {
      return mesh_key_compare(pair.first, key) == 0;
    });

    if (itr == model_submissions.end()) {
      auto& verts = submission.model->GetModelSource()->RawVertices();
      auto& idxs = submission.model->GetModelSource()->Indices();

      itr = InsertMeshKey(key, verts, idxs);
      OE_ASSERT(itr != model_submissions.end(), "Failed to insert mesh key");
    }

    auto& [mk, sl] = *itr;
    sl.cpu_model_storage.BufferData(submission.transform);
    sl.cpu_material_storage.BufferData(submission.material);
    ++sl.instance_count;
  }

  void Pipeline::Render() {
    /** Passes to implement
     * ----------------
     * shadow mapping (expensive) :
     *  direct light map pass (from viewpoint of primary direct light source)
     *  spot shadow map pass (from each pointlight ???)
     *
     * pre depth pass
     * hzb compute
     * pre integration
     * light culling
     * skybox pass
     * GTAO compute
     * GTAO denoise compute
     * AO Composite
     * pre convolution compute
     * jump flood
     * ssr compute
     * ssr composite
     * edge detection
     * bloom compute
     * composite pass
     **/
    material_storage->Clear();
    model_storage->Clear();

    gbuffer.Bind();
    CHECKGL();

    RenderAll();
    CHECKGL();

    gbuffer.Unbind();
    CHECKGL();

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

  Ref<Framebuffer> Pipeline::GetOutput() {
    return target;
  }

  GBuffer& Pipeline::GetGBuffer() {
    return gbuffer;
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

    CHECKGL();

    RenderAll();

    CHECKGL();

    pass->Unbind();
    CHECKGL();
  }

  FrameMeshes::iterator Pipeline::InsertMeshKey(MeshKey& key, const std::vector<float>& vertices, const std::vector<Index>& indices) {
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

    return model_submissions.insert({key, std::move(msl)}).first;
  }

  void Pipeline::RenderAll() {
    for (auto& [mk, sl] : model_submissions) {
      RenderMeshes(mk, sl.instance_count, sl.cpu_model_storage, sl.cpu_material_storage);
    }
  }

  void Pipeline::RenderMeshes(const MeshKey& mesh_key, uint32_t instance_count, const Buffer& model_buffer, const Buffer& material_buffer) {
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

}  // namespace other
