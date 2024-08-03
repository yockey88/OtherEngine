/**
 * \file rendering/render_pass.cpp
 **/
#include "rendering/render_pass.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"

namespace other {
      
  RenderPass::RenderPass(RenderPassSpec spec) 
      : spec(spec) {
    for (auto& u : spec.uniforms) {
      uniforms[FNV(u.name)] = u;
    } 

    const std::vector<Uniform> material_unis = {
      { "voe_model"          , MAT4  } ,
      { "foe_color"          , VEC3 } ,

      { "foe_material.ambient"   , VEC3  } ,
      { "foe_material.diffuse"   , VEC3  } ,
      { "foe_material.specular"  , VEC3  } ,
      { "foe_material.shininess" , FLOAT } ,

      { "foe_plight.position"   , VEC3  } ,
      { "foe_plight.ambient"    , VEC3  } ,
      { "foe_plight.diffuse"    , VEC3  } ,
      { "foe_plight.specular"   , VEC3  } ,
      { "foe_plight.constant"   , FLOAT  } ,
      { "foe_plight.linear"     , FLOAT  } ,
      { "foe_plight.quadratic"  , FLOAT  } ,

      { "foe_dlight.direction"   , VEC3  } ,
      { "foe_dlight.ambient"    , VEC3  } ,
      { "foe_dlight.diffuse"    , VEC3  } ,
      { "foe_dlight.specular"   , VEC3  } ,
      { "foe_dlight.constant"   , FLOAT  } ,
      { "foe_dlight.linear"     , FLOAT  } ,
      { "foe_dlight.quadratic"  , FLOAT  } ,
    }; 
    for (const auto& u : material_unis) {
      DefineInput(u);
    }
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

    /// gamma correction 
    // glEnable(GL_FRAMEBUFFER_SRGB);

    CHECKGL();

    SetRenderState();

    CHECKGL();

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

    auto& buff = uniform_blocks[hash] = uni_buffer;
    buff->BindBase();
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
