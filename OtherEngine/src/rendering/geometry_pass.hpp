/**
 * \file rendering/geometry_pass.hpp
 **/
#ifndef OTHER_ENGINE_GEOMETRY_PASS_HPP
#define OTHER_ENGINE_GEOMETRY_PASS_HPP

#include <vector>

#include "rendering/shader.hpp"
#include "rendering/render_pass.hpp"

namespace other {

  class GeometryPass : public RenderPass {
    public:
      GeometryPass(const std::vector<Uniform>& uniforms , const Ref<Shader>& shader);
      virtual ~GeometryPass() override {}

      virtual void SetRenderState() override;
  };  

} // namespace other

#endif // !OTHER_ENGINE_GEOMETRY_PASS_HPP
