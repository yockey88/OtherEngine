/**
 * \file rendering/render_pass.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_PASS_HPP
#define OTHER_ENGINE_RENDER_PASS_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "core/buffer.hpp"
#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/writer_reader.hpp"

#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"

/** Idea of render-pass chaining for dynamic pipeline construction
 *
 *  - implement a chaining utility to dynamically construct render pass chains
 *      to allow for custom pipelines
 *
 *  - let E = scene-lighting-environment
 *      accessible globally
 *
 *  - renderpass rp : X -> Y
 *      where X is either
 *       1 - a scene mesh/uniform set
 *       2 - a set of render buffers/textures
 *    ex:
 *      let g_rp = gbuffer_pass
 *      then
 *        X = { geometry-data , transforms , materials }
 *        Y_g = { position , normal , albedo , depth }
 *
 *      let l_rp = lighting_pass
 *      then
 *        X = Y_g U { textured-quad-mesh }
 *        Y_l = { lit-scene-texture }
 *
 *      so that this hardcoded pipeline below cane be thought of as
 *        output-frame = l_rp o g_rp = l_rp(g_rp(X))
 *
 *  - a generic pipeline then is
 *      pl : P -> i,
 *        P is a subset of Rp (the set of all renderpasses),
 *          P = { p_0 , ... , p_n }
 *        i \in I (the set of output images of the engine in a single run)
 *
 *      pl : p_n o ... o p_0 = p_n(...p_1(p_0(G))) = i;
 *        G = { geometry-data , transforms , materials }
 **/

namespace other {

  struct RenderPassSpec {
    std::string name = "";
    glm::vec4 tag_col;

    std::vector<Uniform> uniforms;
    Ref<Shader> shader = nullptr;
  };

  class RenderPass : public RefCounted {
   public:
    RenderPass(RenderPassSpec spec);
    virtual ~RenderPass() override {}

    std::string Name() const;

    void Bind();
    Ref<Shader> GetShader();
    void Unbind();

    void DefineInput(const Ref<UniformBuffer>& uniform_block);
    void DefineInput(Uniform uniform);

    template <typename T>
    void SetInput(const std::string_view block_name, const std::string_view name, T val, uint32_t index = 0) {
      uint64_t blck_hash = FNV(block_name);

      auto itr = uniform_blocks.find(blck_hash);
      if (itr == uniform_blocks.end()) {
        OE_ERROR("Failed to set uniform {}, block {} not defined in pass {}", name, block_name, spec.name);
        return;
      }

      auto& [id, uni_blck] = *itr;
      uni_blck->SetUniform(name, val, index);
    }

    /// FIXME: do something about this mess
    template <typename T>
    void SetInput(const std::string_view name, T val, uint32_t index = 0) {
      uint64_t hash = FNV(name);

      auto itr = uniforms.find(hash);
      if (itr == uniforms.end()) {
        OE_ERROR("Failed to set uniform {}, not defined in render pass {}", name, spec.name);
        return;
      }

      auto& [id, uni] = *itr;
      OE_ASSERT(sizeof(T) == GetValueSize(uni.type), "Attempting to set uniform {} to invalidly sized type {}", name, typeid(T).name());
      spec.shader->SetUniform(uni.name, val, index);
    }

    virtual void SetRenderState() {}

   protected:
    std::map<UUID, Uniform> uniforms;
    std::map<UUID, Ref<UniformBuffer>> uniform_blocks;

    RenderPassSpec spec;
  };

  template <>
  struct Writer<RenderPassSpec> {
    std::ostream& operator()(std::ostream& os, const RenderPassSpec& spec) {
      BeginWriteList(os) << "\n    ";
      Writer<std::string>{}(os, spec.name) << "\n    ";
      WriteKeyValue(os, "tag-color", spec.tag_col) << "    ";

      os << "uniforms = ";
      BeginWriteList(os) << "\n      ";
      for (uint32_t i = 0; i < spec.uniforms.size(); i++) {
        WriteListItem(os, spec.uniforms[i], i == spec.uniforms.size() - 1);
        if (i != spec.uniforms.size() - 1) {
          os << ",\n      ";
        } else {
          os << "\n    ";
        }
      }
      EndWriteList(os) << "\n    ";

      if (spec.shader != nullptr) {
        WriteKeyValue(os, "shader", spec.shader->handle.Get());
      } else {
        WriteKeyValue(os, "shader", 0);
      }
      EndWriteList(os) << "\n";

      return os;
    }
  };

  template <>
  struct Reader<RenderPassSpec> {
    RenderPassSpec operator()(std::istream& stream) {
      BeginReadList(stream);
      std::string name_val = Reader<std::string>{}(stream);
      auto [tag_col, tag_col_val] = ReadKeyValue<glm::vec4>(stream, Reader<glm::vec4>{});
      auto [uniforms, uniforms_val] = ReadKeyValue<std::vector<Uniform>>(stream, Reader<std::vector<Uniform>>{});
      EndReadList(stream);

      return RenderPassSpec{
        .name = name_val,
        .tag_col = tag_col_val,
        .uniforms = uniforms_val,
      };
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_RENDER_PASS_HPP
