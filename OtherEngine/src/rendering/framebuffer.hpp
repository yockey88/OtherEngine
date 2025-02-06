/**
 * \file rendering/framebuffer.hpp
 **/
#ifndef OTHER_ENGINE_FRAMEBUFFER_HPP
#define OTHER_ENGINE_FRAMEBUFFER_HPP

#include <cstdint>

#include <glm/glm.hpp>

#include "core/ref_counted.hpp"
#include "core/writer_reader.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  class Framebuffer : public RefCounted {
   public:
    Framebuffer(const FramebufferSpec& spec);
    ~Framebuffer();

    bool Valid() const;

    void Resize(glm::vec2& size);
    glm::vec2 Size() const;

    void BindFrame();
    void UnbindFrame();

    void Draw() const;

    uint32_t depth_attachment = 0;
    uint32_t color_attachment = 0;

    uint32_t texture = 0;

   private:
    friend struct Writer<Framebuffer>;

    uint32_t fbo = 0;
    uint32_t rbo = 0;
    uint32_t framebuffer = 0;

    uint32_t intermediate_fbo = 0;

    bool fb_complete = false;

    uint32_t clear_flags = 0;

    FramebufferSpec spec = {};

    void CreateFramebuffer();
    void DestroyFramebuffer();

    void CreateWithOldTextures();
    void DestroyKeepTextures();
  };

  template <>
  struct Writer<FramebufferSpec> {
    std::ostream& operator()(std::ostream& os, const FramebufferSpec& spec) {
      BeginWriteList(os) << "\n    ";
      Writer<std::string>{}(os, spec.framebuffer_name) << "\n    ";
      WriteKeyValue(os, "depth-function", spec.depth_func) << "    ";
      WriteKeyValue(os, "clear-color", spec.clear_color) << "    ";
      WriteKeyValue(os, "size", spec.size) << "    ";
      WriteKeyValue(os, "depth-buffer", spec.depth) << "    ";
      WriteKeyValue(os, "color-buffer", spec.color) << "    ";
      WriteKeyValue(os, "stencil-buffer", spec.stencil) << "  ";
      EndWriteList(os) << "\n";
      return os;
    }
  };

  template <>
  struct Reader<FramebufferSpec> {
    FramebufferSpec operator()(std::istream& stream) {
      FramebufferSpec spec;
      BeginReadList(stream);

      spec.framebuffer_name = Reader<std::string>{}(stream);

      auto [dfk, depth_func] = ReadKeyValue(stream, Reader<DepthFunction>{});
      spec.depth_func = depth_func;

      auto [cck, clear_color] = ReadKeyValue(stream, Reader<glm::vec4>{});
      spec.clear_color = clear_color;

      auto [szk, size] = ReadKeyValue(stream, Reader<glm::ivec2>{});
      spec.size = size;

      auto [dbk, depth] = ReadKeyValue(stream, Reader<bool>{});
      spec.depth = depth;

      auto [cbk, color] = ReadKeyValue(stream, Reader<bool>{});
      spec.color = color;

      auto [sbk, stencil] = ReadKeyValue(stream, Reader<bool>{});
      spec.stencil = stencil;

      EndReadList(stream);
      return spec;
    }
  };

}  // namespace other

#endif  // !OTHER_ENGINE_FRAMEBUFFER_HPP
