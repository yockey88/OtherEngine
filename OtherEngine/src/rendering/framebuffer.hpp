/**
 * \file rendering/framebuffer.hpp
 **/
#ifndef OTHER_ENGINE_FRAMEBUFFER_HPP
#define OTHER_ENGINE_FRAMEBUFFER_HPP

#include <cstdint>

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  class Framebuffer : public RefCounted {
    public:
      Framebuffer();
      Framebuffer(const FramebufferSpec& spec);
      ~Framebuffer();

      bool Valid() const;

      void Resize(const glm::vec2& size);

      void BindFrame();
      void UnbindFrame();

      void Draw() const;
      
      uint32_t depth_attachment = 0;
      uint32_t color_attachment = 0;

      uint32_t texture = 0;

    private:
      uint32_t fbo = 0;
      uint32_t rbo = 0;
      uint32_t framebuffer = 0;

      uint32_t intermediate_fbo = 0;

      bool fb_complete = false;

      uint32_t clear_flags = 0;

      FramebufferSpec spec = {};

      void CreateFramebuffer();
      void DestroyFramebuffer();
  };

} // namespace other

#endif // !OTHER_ENGINE_FRAMEBUFFER_HPP
