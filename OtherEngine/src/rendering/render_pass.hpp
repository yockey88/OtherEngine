/**
 * \file rendering/render_pass.hpp
 **/
#ifndef OTHER_ENGINE_RENDER_PASS_HPP
#define OTHER_ENGINE_RENDER_PASS_HPP

#include "core/ref_counted.hpp"

namespace other {

  class RenderPass : public RefCounted {
    public:
      virtual ~RenderPass() override {}
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RENDER_PASS_HPP
