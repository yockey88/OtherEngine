/**
 * \file rendering/outline_pass.hpp
 **/
#ifndef OTHER_ENGINE_OUTLINE_PASS_HPP
#define OTHER_ENGINE_OUTLINE_PASS_HPP

#include <vector>

#include "render_pass.hpp"

namespace other {

  class OutlinePass : public RenderPass {
    public:
      OutlinePass(const std::vector<Uniform>& uniforms , const Ref<Shader>& shader);
      virtual ~OutlinePass() override {}

      virtual void SetRenderState() override;
      virtual Buffer ProcessModels(Buffer& buffer) override;
  };

} // namespace other

#endif // !OTHER_ENGINE_OUTLINE_PASS_HPP
