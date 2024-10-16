/**
 * \file rendering/render_pass.cpp
 **/
#include "rendering/render_pass.hpp"

#include <glad/glad.h>

#include "core/defines.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/point_light.hpp"
#include "rendering/direction_light.hpp"

namespace other {
      
  RenderPass::RenderPass(RenderPassSpec spec) 
      : spec(spec) {
    DefineInput({ "goe_position" , SAMPLER2D , sizeof(uint32_t) });
    DefineInput({ "goe_normal"   , SAMPLER2D , sizeof(uint32_t) });
    DefineInput({ "goe_albedo"   , SAMPLER2D , sizeof(uint32_t) });
    for (auto& u : spec.uniforms) {
      DefineInput(u);
    } 

    std::vector<Uniform> light_uniforms = {
      { "direction_light" , USER_TYPE , 1 , sizeof(DirectionLight) } ,
      { "num_point_lights" , INT32 } ,
      { "point_lights" , USER_TYPE , 10 , sizeof(PointLight) } ,
    };
    uint32_t light_binding_point = 2;
    Ref<UniformBuffer> uni_buffer = NewRef<UniformBuffer>("LightData" , light_uniforms , light_binding_point , SHADER_STORAGE);
    DefineInput(uni_buffer);

    const std::vector<Uniform> material_unis = {
      { "materials" , USER_TYPE } ,
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
    
    CHECKGL();
    
    /// reset to sane default for next render pass
    /// depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    CHECKGL();

    /// stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS , 1 , 0xFF);
    glStencilMask(0xFF);
    glStencilOp(GL_KEEP , GL_KEEP , GL_REPLACE);

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
    } else {
      OE_DEBUG("Bound Uniform Buffer : {}" , uni_buffer->Name());
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
