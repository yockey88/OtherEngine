/**
 * \file rendering/pipeline.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_HPP
#define OTHER_ENGINE_PIPELINE_HPP

#include <functional>

#include "core/buffer.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "rendering/framebuffer.hpp"
#include "rendering/gbuffer.hpp"
#include "rendering/layout.hpp"
#include "rendering/material.hpp"
#include "rendering/model.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

  using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle source_handle;
    Ref<VertexArray> vao = nullptr;
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    size_t num_elements = 0;
    bool selected;
  };

}  // namespace other

template <>
struct std::hash<other::MeshKey> {
  std::size_t operator()(const other::MeshKey& key) const {
    return std::hash<uint64_t>{}(key.source_handle.Get()) ^
      std::hash<uint32_t>{}(static_cast<uint32_t>(key.render_state)) ^
      std::hash<uint32_t>{}(static_cast<uint32_t>(key.draw_mode));
  }
};

template <>
struct std::equal_to<other::MeshKey> {
  bool operator()(const other::MeshKey& lhs, const other::MeshKey& rhs) const {
    return lhs.source_handle.Get() == rhs.source_handle.Get() && lhs.render_state == rhs.render_state && lhs.draw_mode == rhs.draw_mode;
  }
};

namespace other {
  struct PipelineSpec {
    DrawMode topology = DrawMode::TRIANGLES;
    bool back_face_culling = true;
    bool depth_test = true;
    float line_width = 1.f;

    FramebufferSpec framebuffer_spec{};
    Layout vertex_layout;

    std::vector<Uniform> model_uniforms{};
    uint32_t model_binding_point = 0;

    std::vector<Uniform> material_uniforms{};
    uint32_t material_binding_point = 0;

    std::string debug_name;
  };

  struct RenderSubmission {
    Ref<Model> model = nullptr;
    glm::mat4 transform = glm::mat4(1.f);
    Material material{};
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    operator MeshKey() const;
  };

  struct MeshSubmissionList {
    uint32_t instance_count = 0;
    Buffer cpu_model_storage;
    Buffer cpu_material_storage;
  };
  using FrameMeshes = std::unordered_map<MeshKey, MeshSubmissionList>;

  class Pipeline : public RefCounted {
   public:
    Pipeline(PipelineSpec& spec);
    virtual ~Pipeline() override {}

    void SetViewportSize(const glm::ivec2& size);

    void SubmitRenderPass(const Ref<RenderPass>& render_pass);

    /// FIXME: material system needs overhaul
    void SubmitModel(Ref<Model> model, const glm::mat4& transform, const Material& color);
    void SubmitStaticModel(Ref<StaticModel> model, const glm::mat4& transform, const Material& color);
    void SubmitStaticModel(const RenderSubmission& submission);

    void Render();
    Ref<Framebuffer> GetOutput();
    GBuffer& GetGBuffer();

    void Clear();

   private:
    uint32_t vao_id = 0;
    PipelineSpec spec{};
    GBuffer gbuffer;

    Ref<UniformBuffer> model_storage = nullptr;
    Ref<UniformBuffer> material_storage = nullptr;
    FrameMeshes model_submissions;

    Ref<Framebuffer> target = nullptr;
    std::vector<Ref<RenderPass>> passes{};

    void PerformPass(Ref<RenderPass>& pass);

    FrameMeshes::iterator InsertMeshKey(MeshKey& key, const std::vector<float>& vertices, const std::vector<Index>& indices);

    void RenderAll();
    void RenderMeshes(const MeshKey& mesh_key, uint32_t instance_count, const Buffer& model_buffer, const Buffer& material_buffer);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PIPELINE_HPP
