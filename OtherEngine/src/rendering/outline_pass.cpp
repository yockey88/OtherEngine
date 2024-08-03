
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
    RegisterUniformProcessor<glm::mat4>("voe_model" , [](glm::mat4& model) {
      float scale = 1.03f;
      model = glm::scale(model , { scale , scale , scale });
    });
  }

  void OutlinePass::SetRenderState() {
    glDisable(GL_DEPTH_TEST);
  }

} // namespace other
