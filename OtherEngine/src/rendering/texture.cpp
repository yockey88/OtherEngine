/**
 * \file rendering/texture.cpp
 **/
#include "rendering/texture.hpp"

#include "core/rand.hpp"

namespace other {

  Texture2D::Texture2D(const Path& file_path) 
      : Texture(file_path) {
    handle = Random::GenerateUUID();
  }

  Texture2D::Texture2D(const glm::ivec2& size , const Buffer& data , ChannelType channels , TargetType type) 
      : Texture(size , data , channels , type) {
    handle = Random::GenerateUUID();
  }
      
  Buffer Texture2D::PixelData() {
    return pixel_data;
  }

  // AssetType CubeMapTexture::GetStaticType() {
  //   return AssetType::CUBE_MAP_TEXTURE;
  // }

  // AssetType CubeMapTexture::GetAssetType() const {
  //   return AssetType::CUBE_MAP_TEXTURE;
  // }

} // namespace other
