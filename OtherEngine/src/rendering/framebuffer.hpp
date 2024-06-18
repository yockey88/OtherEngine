/**
 * \file rendering/framebuffer.hpp
 **/
#ifndef OTHER_ENGINE_FRAMEBUFFER_HPP
#define OTHER_ENGINE_FRAMEBUFFER_HPP

#include <cstdint>

#include "core/ref_counted.hpp"
#include "rendering/vertex.hpp"

namespace other {

  class Framebuffer : public RefCounted {
    public:
      Framebuffer();
      ~Framebuffer();

      uint32_t Texture() const;
      bool Valid() const;

      void Resize(const glm::vec2& size);

      void BindFrame();
      void UnbindFrame();

      void Draw() const;

    private:
      glm::vec2 size;

      uint32_t fbo = 0;
      uint32_t rbo = 0;
      uint32_t framebuffer = 0;

      uint32_t intermediate_fbo = 0;

      uint32_t color_attachment = 0;

      bool fb_complete = false;

      Scope<VertexArray> mesh = nullptr;

      void CreateFramebuffer();
      void DestroyFramebuffer();
  };

} // namespace other

#endif // !OTHER_ENGINE_FRAMEBUFFER_HPP
