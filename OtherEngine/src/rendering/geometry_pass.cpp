/**
 * \file rendering/geometry_pass.cpp
 **/
#include "rendering/geometry_pass.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/uniform.hpp"

namespace other {

  GeometryPass::GeometryPass(const std::vector<Uniform>& uniforms , const Ref<Shader>& shader)
      : RenderPass({
          .name = "GeometryPass" ,
          .tag_col = { 0.f , 0.f , 1.f , 1.f } ,
          .uniforms = uniforms , 
          .shader = shader ,
        }) {
  }

  void GeometryPass::SetRenderState() {
  }

} // namespace other
