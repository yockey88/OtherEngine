/**
 * \file rendering/render_graph.cpp
 **/
#include "rendering/render_graph.hpp"

namespace other {

  void RenderGraph::SubmitSourceNode(const Source& src) {
    source = src;
  }

  UUID RenderGraph::SubmitRenderPass(const std::string_view name, const Ref<RenderPass>& pass) {
    UUID fnv = GetUniqueID(name);
    passes[fnv] = Pass{ .pass = pass };

    return fnv;
  }

  UUID RenderGraph::SubmitTargetNode(const std::string_view name, const Ref<Framebuffer>& target) {
    UUID fnv = GetUniqueID(name);
    targets[fnv] = Target{ .target = target };

    return fnv;
  }

  void RenderGraph::SubmitCompositeFrame(const Ref<Framebuffer>& target) {
  }

  UUID RenderGraph::GetUniqueID(const std::string_view name) {
    UUID fnv = 0;
    std::string name_str = std::string{ name };
    do {
      fnv = FNV(name_str);
      name_str += std::to_string(fnv.Get());
    } while (passes.contains(fnv));

    return fnv;
  }

}  // namespace other
