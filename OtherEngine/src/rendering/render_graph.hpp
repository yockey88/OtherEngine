/**
 * \file rendering/render_graph.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_GRAPH_HPP
#define OTHER_ENGINE_RENDER_GRAPH_HPP

#include <functional>

#include "core/buffer.hpp"

#include "rendering/framebuffer.hpp"
#include "rendering/gbuffer.hpp"
#include "rendering/model.hpp"
#include "rendering/render_pass.hpp"
#include "rendering/vertex.hpp"

namespace other {

  using RenderFn = std::function<void(void*)>;

  struct MeshKey {
    AssetHandle source_handle;
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;
    uint32_t submesh_idx = 0;
  };

}  // namespace other

template <>
struct std::hash<other::MeshKey> {
  std::size_t operator()(const other::MeshKey& key) const {
    return std::hash<uint64_t>{}(key.source_handle.Get()) ^
      std::hash<uint32_t>{}(static_cast<uint32_t>(key.render_state)) ^
      std::hash<uint32_t>{}(static_cast<uint32_t>(key.draw_mode)) ^
      std::hash<uint32_t>{}(key.submesh_idx);
  }
};

template <>
struct std::equal_to<other::MeshKey> {
  bool operator()(const other::MeshKey& lhs, const other::MeshKey& rhs) const {
    return lhs.source_handle.Get() == rhs.source_handle.Get() && lhs.render_state == rhs.render_state && lhs.draw_mode == rhs.draw_mode && lhs.submesh_idx == rhs.submesh_idx;
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
    UUID material{};
    RenderState render_state = RenderState::FILL;
    DrawMode draw_mode = DrawMode::TRIANGLES;

    operator MeshKey() const;
  };

  struct RenderSubmission {
    Ref<Model> model = nullptr;
    glm::mat4 transform = glm::mat4(1.f);
    UUID material{};
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

  class RenderGraph {
   public:
    RenderGraph() {}
    ~RenderGraph() {}

    struct DescriptorSet {
      std::vector<Uniform> uniforms;
    };

    struct Edge {
      DescriptorSet* inputs = nullptr;
      DescriptorSet* outputs = nullptr;
    };

    struct Source {
      FrameMeshes model_submissions;

      Ref<GBuffer> gbuffer = nullptr;
      Ref<UniformBuffer> model_storage = nullptr;
      Ref<UniformBuffer> material_storage = nullptr;

      Ref<MaterialTable> material_table = nullptr;

      std::vector<Edge> outputs;
    };

    struct Pass {
      std::vector<Edge> inputs;
      std::vector<Edge> outputs;
      Ref<RenderPass> pass = nullptr;
    };

    struct Composite {
      std::vector<Edge> inputs;
      Ref<Framebuffer> target = nullptr;
    };

    /// SubmitRenderPass() - Submit a render pass to the render graph
    /// BindRenderResource(in..., out....) - Bind a render resource to the render graph
    /// SubmitTargetSpecification() - Submit a target specification to the render graph

    /// Execute() - Execute the render graph
  };

}  // namespace other

#endif  // !OTHER_ENGINE_RENDER_GRAPH_HPP
