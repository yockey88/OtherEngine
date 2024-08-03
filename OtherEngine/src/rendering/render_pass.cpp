/**
 * \file rendering/render_pass.cpp
 **/
#include "rendering/render_pass.hpp"

#include <glad/glad.h>

namespace other {
      
  RenderPass::RenderPass(RenderPassSpec spec) 
      : spec(spec) {
    for (auto& u : spec.uniforms) {
      uniforms[other::FNV(u.name)] = u;
    } 

    uniforms[other::FNV("voe_model")] = Uniform {
      "voe_model" , other::ValueType::MAT4 
    };
  }

  std::string RenderPass::Name() const {
    return spec.name;
  }
      
  void RenderPass::Bind() {
    if (spec.shader == nullptr) {
      OE_ERROR("RenderPass [{}] has null shader!" , spec.shader);
      return;
    }
    
    /// reset to sane default for next render pass
    /// depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    /// stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS , 1 , 0xFF);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP , GL_KEEP , GL_REPLACE);

    SetRenderState();

    spec.shader->Bind();
  }

  Ref<Shader> RenderPass::GetShader() {
    return spec.shader;
  }
      
  void RenderPass::Unbind() {
    if (spec.shader == nullptr) {
      return;
    }

    spec.shader->Unbind();
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
    if (uniforms.find(hash) != uniforms.end()) {
      OE_ERROR("Can not redefine uniform block {}" , uniform.name);
      return;
    }
  
    uniforms[hash] = uniform;
    
  }

} // namespace other
