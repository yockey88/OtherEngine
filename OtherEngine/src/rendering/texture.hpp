/**
 * \file rendering/texture.hpp
 **/
#ifndef OTHER_ENGINE_TEXTURE_HPP
#define OTHER_ENGINE_TEXTURE_HPP

#include <glad/glad.h>

#include "core/buffer.hpp"

#include "asset/asset.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  template <typename T>
  struct TexParam {
    union {
      T s_val;
      T min;
    };

    union {
      T t_val;
      T mag;
    };

    T r_val;
  };

  struct TextureSpecification {
    ChannelType channels;
    TargetType type;

    glm::ivec2 size;

    TexParam<FilterType> filters;
    TexParam<TextureWrap> wrap;

    bool generate_mipmaps = false;
    bool srgb = false;
    bool storag = false;
    bool store_locally = false;

    std::string name;
  };

  class Texture : public Asset {
   public:
    OE_ASSET(TEXTURE);

    Texture(TargetType type, const Path& path, const TextureSpecification& spec);
    virtual ~Texture() override {}

    void Bind(uint32_t texture_slot = 0) const;
    void Unbind() const;

    uint32_t GetRendererId() const;

    Buffer PixelData();
    Buffer PixelData() const;

   protected:
    TargetType type;
    TextureSpecification spec;
    uint32_t renderer_id = 0;

    Buffer pixel_data;

    void SetChannels(uint32_t format);
    virtual void SetData(const uint8_t* pixels, const glm::vec2& size, int32_t format) = 0;
  };

  class Texture2D : public Texture {
   public:
    Texture2D(const Path& path, const TextureSpecification& spec);
    Texture2D(uint8_t* pixels, const glm::vec2& size);
    Texture2D(const glm::vec3& color, const glm::ivec2& size);
    virtual ~Texture2D() override {}

   private:
    virtual void SetData(const uint8_t* pixels, const glm::vec2& size, int32_t format) override;
  };

  class CubeMapTexture : public Texture {
   public:
    CubeMapTexture(const Path& path, const TextureSpecification& spec);
    virtual ~CubeMapTexture() override {}

   private:
    virtual void SetData(const uint8_t* pixels, const glm::vec2& size, int32_t format) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_TEXTURE_HPP
