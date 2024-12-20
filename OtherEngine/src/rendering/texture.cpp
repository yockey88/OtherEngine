/**
 * \file rendering/texture.cpp
 **/
#include "rendering/texture.hpp"

#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "core/rand.hpp"

namespace other {

  Texture::Texture(TargetType type, const Path& file_path, const TextureSpecification& spec)
      : type(type), spec(spec) {
    int32_t format_detection = 0;
    int32_t w, h;

    // stbi_set_flip_vertically_on_load(true);
    uint8_t* pixels_raw = stbi_load(file_path.string().c_str(), &w, &h, &format_detection, 0);
    OE_ASSERT(pixels_raw != nullptr, "Failed to load image {}", file_path);

    SetChannels(format_detection);
    glGenTextures(1, &renderer_id);
    Bind();

    glTexImage2D(type, 0, spec.channels, w, h, 0, spec.channels, GL_UNSIGNED_BYTE, pixels_raw);
    glGenerateMipmap(type);

    stbi_image_free(pixels_raw);
  }

  void Texture::Bind(uint32_t slot) const {
    glBindTexture(type, renderer_id);
  }

  void Texture::Unbind() const {
    glBindTexture(type, 0);
  }

  uint32_t Texture::GetRendererId() const {
    return renderer_id;
  }

  Buffer Texture::PixelData() {
    return pixel_data;
  }

  Buffer Texture::PixelData() const {
    return pixel_data;
  }

  void Texture::SetChannels(uint32_t format) {
    switch (spec.channels) {
      case RED: spec.channels = RED; break;
      case RGB: spec.channels = RGB; break;
      case RGBA: spec.channels = RGBA; break;
      default:
        OE_ASSERT(false, "INVALID IMAGE FORMAT {} on image {}", spec.channels, spec.name);
    }
  }

  Texture2D::Texture2D(const Path& file_path, const TextureSpecification& spec)
      : Texture(TEX_2D, file_path, spec) {
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, spec.filters.min);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, spec.filters.mag);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, spec.wrap.s_val);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, spec.wrap.t_val);
  }

  Texture2D::Texture2D(uint8_t* pixels, const glm::vec2& size)
      : Texture(TEX_2D, Path(""), TextureSpecification{}) {
    spec.size = size;
    SetData(pixels, size, spec.channels);
  }

  Texture2D::Texture2D(const glm::vec3& color, const glm::ivec2& size)
      : Texture(TEX_2D, Path(""), TextureSpecification{}) {
    spec.size = size;

    std::vector<uint8_t> pixels{};
    pixels.resize(size.x * size.y * 3);

    for (int i = 0; i < size.x * size.y; i++) {
      pixels[i * 3] = static_cast<uint8_t>(color.r * 255);
      pixels[i * 3 + 1] = static_cast<uint8_t>(color.g * 255);
      pixels[i * 3 + 2] = static_cast<uint8_t>(color.b * 255);
    }

    SetData(pixels.data(), size, RGB);
  }

  void Texture2D::SetData(const uint8_t* pixels, const glm::vec2& size, int32_t format) {
    SetChannels(format);

    glGenTextures(1, &renderer_id);
    Bind();

    glTexImage2D(type, 0, spec.channels, size.x, size.y, 0, spec.channels, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(type);

    Unbind();
  }

  CubeMapTexture::CubeMapTexture(const Path& path, const TextureSpecification& spec)
      : Texture(TEX_CUBE_MAP, path, spec) {
  }

  void CubeMapTexture::SetData(const uint8_t* pixels, const glm::vec2& size, int32_t format) {
  }

}  // namespace other
