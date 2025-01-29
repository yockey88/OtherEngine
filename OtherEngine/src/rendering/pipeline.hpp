/**
 * \file rendering/pipeline.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_HPP
#define OTHER_ENGINE_PIPELINE_HPP

#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/writer_reader.hpp"

#include "rendering/draw_calls.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/model.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {

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
    void SubmitModel(const Ref<Model>& model, const Ref<MaterialTable>& material_table, const glm::mat4& transform, UUID material_id, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitModel(const RenderSubmission& submission);

    void SubmitStaticModel(const Ref<StaticModel>& model, const Ref<MaterialTable>& material_table, const glm::mat4& transform, UUID material_id, DrawMode topology = DrawMode::TRIANGLES);
    void SubmitStaticModel(const RenderStaticSubmission& submission);

    void SubmitDebugDrawCommands(const std::vector<DebugDrawCommand>& cmds);

    void Render();
    Ref<Framebuffer> GetOutput() const;

    void Clear();

   private:
    uint32_t vao_id = 0;
    PipelineSpec spec{};

    std::vector<DebugDrawCommand> debug_draw_commands;

    FrameMeshes model_submissions;
    StaticFrameMeshes static_model_submissions;
    std::map<MeshKey, DrawCall> draw_calls;

    Ref<UniformBuffer> model_storage = nullptr;
    Ref<UniformBuffer> material_storage = nullptr;

    Ref<Framebuffer> target = nullptr;
    std::vector<Ref<RenderPass>> passes{};

    void PerformPass(Ref<RenderPass>& pass);

    FrameMeshes::iterator InsertMeshKey(MeshKey& key, const Ref<Model>& model);
    StaticFrameMeshes::iterator InsertStaticMeshKey(MeshKey& key, const Ref<StaticModel>& model);

    void SubmitDrawCall(const MeshKey& key, const DrawCall& call);

    void RenderAll();
    void RenderStaticMeshes(const MeshKey& mesh_key, StaticMeshDrawCall& sl);
    void RenderMeshes(const MeshKey& mesh_key, MeshDrawCall& sl);
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
