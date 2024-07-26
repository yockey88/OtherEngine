/**
 * \file rendering/texture.cpp
 **/
#include "rendering/texture.hpp"
#include <glad/glad.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include "core/rand.hpp"

namespace other {
  
  Texture::Texture(TargetType type , const Path& file_path , const TextureSpecification& spec)
      : type(type) , spec(spec) {
    handle = Random::GenerateUUID();

    int32_t format_detection = 0;
    int32_t w , h;

    // stbi_set_flip_vertically_on_load(true);
    uint8_t* pixels_raw = stbi_load(file_path.string().c_str() , &w , &h , &format_detection , 0);
    OE_ASSERT(pixels_raw != nullptr , "Failed to load image {}" , file_path);

    SetData(pixels_raw , { w , h } , format_detection);
  }
  
  void Texture::Bind(uint32_t slot) const {
    glBindTexture(type , renderer_id);
  }
  
  void Texture::Unbind() const {
    glBindTexture(type , 0);
  }
      
  uint32_t Texture::GetRendererId() const {
    return renderer_id;
  }
  
  Buffer Texture::PixelData() {
    return pixel_data;
  }
      
  void Texture::SetData(uint8_t* pixels , const glm::vec2& size , int32_t format) {
    // pixel_data.Write(pixels , size.x * size.y);

    switch (format) {
      case 1: spec.channels = RED; break;
      case 3: spec.channels = RGB; break;
      case 4: spec.channels = RGBA; break;
      default:
        OE_ASSERT(false , "INVALID IMAGE FORMAT {} on image {}" , format , spec.name);
    }

    glGenTextures(1 , &renderer_id);
    Bind();

    OnLoad();
    glTexImage2D(type , 0 , spec.channels , size.x , size.y , 0 , spec.channels , GL_UNSIGNED_BYTE , pixels);
    glGenerateMipmap(type);
    stbi_image_free(pixels);

    Unbind();
  }

  Texture2D::Texture2D(const Path& file_path , const TextureSpecification& spec) 
      : Texture(TEX_2D , file_path , spec) {
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, spec.filters.min);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, spec.filters.mag);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, spec.wrap.s_val);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, spec.wrap.t_val);
  }
      
  void Texture2D::OnLoad() {
  }
      
  CubeMapTexture::CubeMapTexture(const Path& path , const TextureSpecification& spec) 
      : Texture(TEX_CUBE_MAP , path , spec) {
  }

} // namespace other
