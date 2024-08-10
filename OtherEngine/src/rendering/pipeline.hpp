/**
 * \file rendering/pipeline.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_HPP
#define OTHER_ENGINE_PIPELINE_HPP

#include <functional>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/model.hpp"
#include "rendering/material.hpp"
#include "rendering/render_pass.hpp"

namespace other {

  using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle model_handle;
    // AssetHandle material_handle;
    Ref<VertexArray> vao = nullptr;
    size_t num_elements = 0;
    bool selected;
  };

} // namespace other

namespace std {

  template <>
  struct less<other::MeshKey> {
    bool operator()(const other::MeshKey& lhs , const other::MeshKey& rhs) const;
  };

} // namespace std

namespace other {

  struct PipelineSpec {
    DrawMode topology = DrawMode::TRIANGLES;
    bool back_face_culling = true;
    bool depth_test = true;
    bool depth_write = true;
    bool wire_frame = false;
    float line_width = 1.f;

    FramebufferSpec framebuffer_spec {};
    Layout vertex_layout;

    Ref<UniformBuffer> model_storage;

    std::string debug_name;
  };

  class Pipeline : public RefCounted {
    public:
      Pipeline(PipelineSpec& spec);
      virtual ~Pipeline() override {}
      
      void SubmitRenderPass(const Ref<RenderPass>& pass);

      /// Add materials to model submission
      void SubmitModel(Ref<Model>& model , const glm::mat4& transform , const Material& color);
      void SubmitStaticModel(Ref<StaticModel>& model , const glm::mat4& transform , const Material& color);

      void Render();
      Ref<Framebuffer> GetOutput();
      
    private:
      uint32_t vao_id = 0;
      PipelineSpec spec{};

      struct RenderSubmission {
        Material material;
      };

      std::map<MeshKey , std::vector<RenderSubmission>> model_submissions;
      
      std::vector<float> vertices{};
      std::vector<uint32_t> indices{};

      uint32_t curr_transform_idx = 0;
      uint32_t idx_offset = 0;
       
      Ref<Framebuffer> target = nullptr;

      std::vector<Ref<RenderPass>> passes;

      void AddIndices(const Index& idxs); 

      void PerformPass(Ref<RenderPass>& pass);
  };

} // namespace other

#endif // !OTHER_ENGINE_PIPELINE_HPP
