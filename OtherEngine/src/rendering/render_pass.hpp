/**
 * \file rendering/render_pass.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_PASS_HPP
#define OTHER_ENGINE_RENDER_PASS_HPP

#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "rendering/shader.hpp"
#include "rendering/uniform.hpp"

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

      void Bind();
      Ref<Shader> GetShader();
      void Unbind();

      void DefineInput(const Ref<UniformBuffer>& uniform_block);
      void DefineInput(Uniform uniform);
      
      template <typename T>
      void SetInput(const std::string_view block_name , const std::string_view name , const T& val) {
        uint64_t blck_hash = FNV(block_name);

        auto itr = uniform_blocks.find(blck_hash);
        if (itr == uniform_blocks.end()) {
          OE_ERROR("Failed to set uniform {}, block {} not defined in pass {}" , name , block_name , spec.name);
          return;
        }

        auto& [id , uni_blck] = *itr;
        uni_blck->SetUniform(name , val);
      }

      template <typename T>
      void SetInput(const std::string_view name , const T& val) {
        uint64_t hash = FNV(name);

        auto itr = uniforms.find(hash);
        if (itr == uniforms.end()) {
          OE_ERROR("Failed to set uniform {}, not defined in render pass {}" , name , spec.name);
          return;
        }

        auto& [id , uni] = *itr;
        if (sizeof(T) != GetValueSize(uni.type)) {
          OE_ERROR("Attempting to set uniform {} to invalidly sized type {}" , name , typeid(T).name());
          return;
        }

        spec.shader->SetUniform(uni.name , val);
      }
      
      // void Input(Storage buffer set) = 0;
      // void Input(Storage buffer) = 0;

      // void Input(TextureXXX set);
      // void Input(TextureXXX);

    private:
      std::map<UUID , Uniform> uniforms;
      std::map<UUID , Ref<UniformBuffer>> uniform_blocks;

      RenderPassSpec spec;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RENDER_PASS_HPP
