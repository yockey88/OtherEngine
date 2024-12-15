/**
 * \file rendering/pipeline.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_HPP
#define OTHER_ENGINE_PIPELINE_HPP

#include <functional>

#include "core/buffer.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/writer_reader.hpp"

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
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

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
    bool back_face_culling = true;
    bool depth_test = true;
    float line_width = 1.f;

    FramebufferSpec framebuffer_spec{};

    std::string pipeline_name;
  };

  struct RenderStaticSubmission {
    Ref<StaticModel> model = nullptr;
    glm::mat4 transform = glm::mat4(1.f);
    Material material{};
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    operator MeshKey() const;
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
    Ref<VertexArray> vao = nullptr;
    size_t num_elements = 0;

    uint32_t instance_count = 0;
    uint32_t base_vertex = 0;
    uint32_t base_instance = 0;

    Buffer cpu_model_storage;
    Buffer cpu_material_storage;
  };
  using FrameMeshes = std::unordered_map<MeshKey, MeshSubmissionList>;

  class Pipeline : public RefCounted {
   public:
    Pipeline(PipelineSpec& spec);
    virtual ~Pipeline() override {}

    std::string Name() const;
    const FramebufferSpec& TargetSpec() const;

    const std::vector<Ref<RenderPass>>& GetRenderPasses() const;

    void SetViewportSize(const glm::ivec2& size);

    void SubmitRenderPass(const Ref<RenderPass>& render_pass);

    /// FIXME: material system needs overhaul
    void SubmitModel(const Ref<Model>& model, const glm::mat4& transform, const Material& color, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitModel(const RenderSubmission& submission);

    void SubmitStaticModel(const Ref<StaticModel>& model, const glm::mat4& transform, const Material& color, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitStaticModel(const RenderStaticSubmission& submission);

    void Render(bool render_gbuffer = false);
    Ref<Framebuffer> GetOutput() const;
    GBuffer& GetGBuffer();

    void Clear();

   private:
    uint32_t vao_id = 0;
    PipelineSpec spec{};
    FrameMeshes model_submissions;

    Ref<GBuffer> gbuffer = nullptr;
    Ref<UniformBuffer> model_storage = nullptr;
    Ref<UniformBuffer> material_storage = nullptr;

    Ref<Framebuffer> target = nullptr;
    std::vector<Ref<RenderPass>> passes{};

    void PerformPass(Ref<RenderPass>& pass);

    FrameMeshes::iterator InsertMeshKey(MeshKey& key, const Ref<Model>& indices);
    FrameMeshes::iterator InsertStaticMeshKey(MeshKey& key, const Ref<StaticModel>& model);

    void RenderAll();
    void RenderMeshes(const MeshKey& mesh_key, MeshSubmissionList& msl);
  };

  template <>
  struct Writer<PipelineSpec> {
    std::ostream& operator()(std::ostream& os, const PipelineSpec& spec) {
      BeginWriteList(os) << "\n    ";
      Writer<std::string>{}(os, spec.pipeline_name) << "\n    ";
      WriteKeyValue(os, "back-face-culling", spec.back_face_culling) << "    ";
      WriteKeyValue(os, "depth-test", spec.depth_test) << "    ";
      WriteKeyValue(os, "line-width", spec.line_width) << "    ";
      WriteKeyValue(os, "framebuffer-spec", spec.framebuffer_spec) << "    ";
      EndWriteList(os) << "\n";
      return os;
    }
  };

  template <>
  struct Reader<PipelineSpec> {
    PipelineSpec operator()(std::istream& stream) {
      PipelineSpec spec;
      BeginReadList(stream);
      spec.pipeline_name = Reader<std::string>{}(stream);

      auto [bfck, back_face_culling] = ReadKeyValue(stream, Reader<bool>{});
      spec.back_face_culling = back_face_culling;

      auto [dtk, depth_test] = ReadKeyValue(stream, Reader<bool>{});
      spec.depth_test = depth_test;

      auto [lwk, line_width] = ReadKeyValue(stream, Reader<float>{});
      spec.line_width = line_width;

      auto [fbk, framebuffer_spec] = ReadKeyValue(stream, Reader<FramebufferSpec>{});
      spec.framebuffer_spec = framebuffer_spec;
      EndReadList(stream);
      return spec;
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_PIPELINE_HPP
