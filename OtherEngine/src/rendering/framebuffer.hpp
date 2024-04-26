/**
 * \file rendering/framebuffer.hpp
 **/
#ifndef OTHER_ENGINE_FRAMEBUFFER_HPP
#define OTHER_ENGINE_FRAMEBUFFER_HPP

#include <cstdint>

namespace other {

  class Framebuffer {
    public:
      Framebuffer();
      ~Framebuffer();

      uint32_t Texture() const;
      bool Valid() const;

      void BindFrame();
      void UnbindFrame();

    private:
      uint32_t fbo = 0;
      uint32_t rbo = 0;
      uint32_t framebuffer = 0;

      uint32_t intermediate_fbo = 0;
      uint32_t screen_texture = 0;

      uint32_t color_attachment = 0;

      bool fb_complete = false;
  };

} // namespace other

#endif // !OTHER_ENGINE_FRAMEBUFFER_HPP
