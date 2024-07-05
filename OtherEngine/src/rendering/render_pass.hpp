/**
 * \file rendering/render_pass.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_PASS_HPP
#define OTHER_ENGINE_RENDER_PASS_HPP

#include <string>

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "rendering/pipeline.hpp"

namespace other {

  struct Uniform {
    std::string name = "";
  };

  struct RenderPassSpec {
    std::string name = "";
    glm::vec4 tag_col;
  };

  class RenderPass : public RefCounted {
    public:
      RenderPass(RenderPassSpec spec) 
        : name(spec.name) , tag_col(spec.tag_col) {}
      virtual ~RenderPass() override {}

      virtual void DefineUniforms();
      virtual void InputUniforms();

      // virtual void Input(Uniform buffer_set) = 0;
      virtual void Input(Uniform buffer) = 0;
      
      // virtual void Input(Storage buffer set) = 0;
      // virtual void Input(Storage buffer) = 0;

      // virtual void Input(TextureXXX set);
      // virtual void Input(TextureXXX);

      // get target buffer
      // get rlvnt pipeline
    private:
      std::string name = "";
      glm::vec4 tag_col;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RENDER_PASS_HPP
