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
#include "rendering/render_pass.hpp"

namespace other {

  using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle model_handle;
    // AssetHandle material_handle;
    glm::mat4 transform = glm::mat4(1.f);
    uint32_t model_idx = 0;
    uint32_t submesh_idx = 0;
    bool selected;
  };

} // namespace other

namespace std {

  template <>
  struct std::less<other::MeshKey> {
    bool operator()(const other::MeshKey& lhs , const other::MeshKey& rhs) const {
      if (lhs.model_handle.Get() < rhs.model_handle.Get()) {
        return true;
      }

      if (lhs.model_handle.Get() > rhs.model_handle.Get()) {
        return false;
      }
      
      if (lhs.submesh_idx < rhs.submesh_idx) {
        return true;
      }

      if (lhs.submesh_idx > rhs.submesh_idx) {
        return false;
      }
      
      /* 
      if (lhs.material_handle.Get() < rhs.material_handle.Get()) {
        return true;
      }

      if (lhs.material_handle.Get() > rhs.material_handle.Get()) {
        return false;
      }
      */

      return lhs.selected && !rhs.selected;
    }
  };

} // namespace std

namespace other {

  struct PipelineSpec {
    bool has_indices = false;
    uint32_t buffer_cap = 4096;

    DrawMode topology = DrawMode::TRIANGLES;
    bool back_face_culling = true;
    bool depth_test = true;
    bool depth_write = true;
    bool wire_frame = false;
    float line_width = 1.f;

    FramebufferSpec framebuffer_spec {};
    Layout vertex_layout;

    std::string debug_name;
  };

  class Pipeline : public RefCounted {
    public:
      Pipeline(const PipelineSpec& spec);
      virtual ~Pipeline() override {}
      
      void SubmitRenderPass(Ref<RenderPass>& pass);

      /// Add materials to model submission
      void SubmitModel(Ref<Model>& model , uint32_t transform_idx);
      void SubmitStaticModel(Ref<StaticModel>& model , uint32_t transform_idx);

      void Render();
      Ref<Framebuffer> GetOutput();
      
    private:
      uint32_t vao_id = 0;
      PipelineSpec spec{};
      
      std::vector<float> vertices{};
      std::vector<uint32_t> indices{};
      
      Scope<VertexBuffer> vertex_buffer = nullptr;
      Scope<VertexBuffer> index_buffer = nullptr;
       
      Ref<Framebuffer> target = nullptr;

      std::vector<Ref<RenderPass>> passes;

      void PerformPass(Ref<RenderPass>& pass);
  };

} // namespace other

#endif // !OTHER_ENGINE_PIPELINE_HPP
