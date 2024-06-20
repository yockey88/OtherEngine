/**
 * \file rendering/texture.hpp
 **/
#ifndef OTHER_ENGINE_TEXTURE_HPP
#define OTHER_ENGINE_TEXTURE_HPP

#include <glad/glad.h>

#include "core/buffer.hpp"
#include "asset/asset.hpp"

namespace other {

  enum FilterType {
    NEAREST = GL_NEAREST ,
    LINEAR = GL_LINEAR
  };

  enum TextureType {
    DIFFUSE , 
    SPECULAR ,
    NORMAL ,
    HEIGHT
  };
  
  enum ChannelType {
    RED = GL_RED ,
    RGB = GL_RGB ,
    RGBA = GL_RGBA
  };
  
  enum TargetType {
    TEX_1D = GL_TEXTURE_1D ,
    TEX_2D = GL_TEXTURE_2D ,
    TEX_3D = GL_TEXTURE_3D ,
    TEX_1D_ARR = GL_TEXTURE_1D_ARRAY ,
    TEX_2D_ARR = GL_TEXTURE_2D_ARRAY ,
    TEX_RECT = GL_TEXTURE_RECTANGLE ,
    TEX_CUBE_MAP = GL_TEXTURE_CUBE_MAP ,
    TEX_CUBE_MAP_ARR = GL_TEXTURE_CUBE_MAP_ARRAY ,
    TEX_BUFFER = GL_TEXTURE_BUFFER ,
    TEX_2D_MULTISAMPLE = GL_TEXTURE_2D_MULTISAMPLE ,
    TEX_2D_MULTISAMPLE_ARR = GL_TEXTURE_2D_MULTISAMPLE_ARRAY
  };

  class Texture : public Asset {
    public:
      Texture(const Path& file_path);
      Texture(const glm::ivec2& size , const Buffer& data , ChannelType channels = RGB ,
              TargetType = TEX_2D);
      virtual ~Texture() override {}

      void Bind(uint32_t texture_slot = 0) const; 
      void Unbind() const;

    private:
  };

  class Texture2D : public Texture {
    public:
      OE_ASSET(TEXTURE2D);

      Texture2D(const Path& file_path);
      Texture2D(const glm::ivec2& size , const Buffer& data , ChannelType channels = RGB ,
              TargetType = TEX_2D);
      virtual ~Texture2D() override {}

      Buffer PixelData();

    private:
      Buffer pixel_data;
  };

  // class CubeMapTexture : public Texture {
  //   public:
  //     CubeMapTexture(const Path& file_path);
  //     CubeMapTexture(const glm::ivec2& size , const Buffer& data , ChannelType channels = RGB ,
  //                    TargetType = TEX_2D);
  //     virtual ~CubeMapTexture() override {}
  //     
  //     static AssetType GetStaticType();
  //     virtual AssetType GetAssetType() const override;

  //   private:
  // };

} // namespace other

#endif // !OTHER_ENGINE_TEXTURE_HPP
