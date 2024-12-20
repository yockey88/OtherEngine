/**
 * \file rendering/render_graph.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_GRAPH_HPP
#define OTHER_ENGINE_RENDER_GRAPH_HPP

#include "rendering/draw_calls.hpp"
#include "rendering/framebuffer.hpp"
#include "rendering/gbuffer.hpp"
#include "rendering/material_table.hpp"
#include "rendering/render_pass.hpp"

namespace other {

  class RenderGraph {
   public:
    RenderGraph() {}
    ~RenderGraph() {}

    // struct DescriptorSet {
    //   UUID id = 0;
    //   std::map<UUID, Uniform> uniforms;
    //   std::map<UUID, Ref<UniformBuffer>> uniform_blocks;
    // };

    struct Edge {
      // DescriptorSet* inputs = nullptr;
      // DescriptorSet* outputs = nullptr;

      // void BindInputs(Ref<RenderPass>& pass);
      // void BindOutputs(Ref<RenderPass>& pass);
    };

    struct Source {
      FrameMeshes model_submissions;

      Ref<GBuffer> gbuffer = nullptr;
      Ref<UniformBuffer> model_storage = nullptr;
      Ref<UniformBuffer> material_storage = nullptr;

      Ref<MaterialTable> material_table = nullptr;

      std::vector<Edge> outputs;
    };

    struct Target {
      UUID id = 0;
      std::vector<Edge> inputs;
      Ref<Framebuffer> target = nullptr;
    };

    struct Pass {
      UUID id = 0;
      std::vector<Edge> inputs;
      Ref<RenderPass> pass = nullptr;
      std::vector<Edge> outputs;
    };

    struct Composite {
      std::vector<Edge> inputs;
      Ref<Framebuffer> target = nullptr;
    };

    void SubmitSourceNode(const Source& src);

    UUID SubmitRenderPass(const std::string_view name, const Ref<RenderPass>& pass);
    UUID SubmitTargetNode(const std::string_view name, const Ref<Framebuffer>& target);

    void SubmitCompositeFrame(const Ref<Framebuffer>& target);

    void Finalize() {}

    /// BindPassResource(pass, in..., out....) - Bind a render resource to the render graph
    /// BindTargetResource(target, in..., out...) - Bind a target resource to the render graph

    /// Execute() - Execute the render graph

   private:
    /// geometry/material/lighting data
    Opt<Source> source = std::nullopt;

    std::map<UUID, Pass> passes;
    std::map<UUID, Target> targets;

    /// final output image
    Composite target = {};

    UUID GetUniqueID(const std::string_view name);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_RENDER_GRAPH_HPP
