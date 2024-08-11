
/**
 * \file rendering/outline_pass.cpp
 **/
#include "rendering/outline_pass.hpp"

#include <glad/glad.h>

namespace other {

  OutlinePass::OutlinePass(const std::vector<Uniform>& uniforms , const Ref<Shader>& shader)
      : RenderPass({
            .name = "ObjectOutlinePass" ,
            .tag_col = { 1.f , 0.2f , 1.f , 1.f } ,
            .uniforms = uniforms ,
            .shader = shader ,
          }) {
  }

  void OutlinePass::SetRenderState() {
    glDisable(GL_DEPTH_TEST);
  }
      
  Buffer OutlinePass::ProcessModels(Buffer& buffer) {
    Buffer res;
    res.Allocate(buffer.Size());

    /// TODO: make this customizeable
    const float scale = 1.03f;
    for (size_t m = 0; m < buffer.NumElements(); ++m) {
      glm::mat4 model = buffer.At<glm::mat4>(m);
      model = glm::scale(model , { scale , scale , scale });
      res.BufferData(model);
    }

    return res;
  }

} // namespace other
