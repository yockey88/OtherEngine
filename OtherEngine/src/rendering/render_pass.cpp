/**
 * \file rendering/render_pass.cpp
 **/
#include "rendering/render_pass.hpp"

namespace other {

  Ref<Shader> RenderPass::GetShader() {
    return spec.shader;
  }
      
  void RenderPass::DefineInput(const Ref<UniformBuffer>& uni_buffer) {
    auto hash = FNV(uni_buffer->Name());
    if (uniform_blocks.find(hash) != uniform_blocks.end()) {
      OE_ERROR("Can not redefine uniform block {}" , uni_buffer->Name());
      return;
    }
  
    uniform_blocks[hash] = uni_buffer;
  }

  void RenderPass::DefineInput(Uniform uniform) {
    auto hash = FNV(uniform.name);
    if (uniform_blocks.find(hash) != uniform_blocks.end()) {
      OE_ERROR("Can not redefine uniform block {}" , uniform.name);
      return;
    }
  
    uniforms[hash] = uniform;
    
  }

} // namespace other
