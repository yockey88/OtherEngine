/**
 * \file rendering/pipeline.hpp
 **/
#ifndef OTHER_ENGINE_PIPELINE_HPP
#define OTHER_ENGINE_PIPELINE_HPP

#include <functional>
#include <map>

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"
#include "rendering/camera_base.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/model.hpp"

namespace other {

  using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle model_handle;
    // AssetHandle material_handle;
    uint32_t submesh_idx = 0;
    bool selected;
  };

} // namespace other

namespace std {

  template <>
  struct std::less<other::MeshKey> {
    bool operator()(const other::MeshKey& lhs , const other::MeshKey& rhs) {
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
  
  struct DrawCommand {
    Ref<Model> model;
    uint32_t submesh_idx = 0;
    // mat table and material

    uint32_t inst_count = 0;
    uint32_t inst_offset = 0;
    bool rigged = false;
  };

  struct StaticDrawCommand {
    Ref<StaticModel> model;
    uint32_t submesh_idx = 0;
    // mat table and material

    uint32_t inst_count = 0;
    uint32_t inst_offset = 0;
  };

  class Pipeline : public RefCounted {
    public:
      Pipeline(const PipelineSpec& spec);
      virtual ~Pipeline() override {}

      void SubmitModel(Ref<Model> model , const glm::mat4& transform);
      void SubmitStaticModel(Ref<StaticModel> model , const glm::mat4& transform);

      /// really this should take in a handle to a uniform buffer
      void BeginRenderPass(Ref<CameraBase> camera , Ref<RenderPass> render_pass);
      Ref<Framebuffer> EndRenderPass();

    private:
      PipelineSpec spec{};
      uint32_t stride;

      Ref<CameraBase> viewpoint = nullptr;
      Ref<Framebuffer> target = nullptr;

      Ref<RenderPass> current_pass = nullptr;

      uint32_t vao_id = 0;
      uint32_t vbo_id = 0;
      Scope<VertexBuffer> vertex_buffer = nullptr;
      Scope<VertexBuffer> index_buffer = nullptr;
  };

} // namespace other

#endif // !OTHER_ENGINE_PIPELINE_HPP
