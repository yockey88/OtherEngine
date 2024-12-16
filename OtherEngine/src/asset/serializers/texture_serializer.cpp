/**
 * \file asset/serializers/texture_serializer.cpp
 **/
#include "asset/serializers/texture_serializer.hpp"

#include "rendering/texture.hpp"

namespace other {

  void TextureSerializer::Serialize(const AssetMetadata& metadata) {
  }

  bool TextureSerializer::Load(AssetMetadata& metadata) {
    /// FIXME: detect and distinguish between texture types

    TextureSpecification spec{
      .channels = RGBA,
      .type = TEX_2D,
      .filters = {
        .min = NEAREST,
        .mag = NEAREST,
      },
      .wrap = {
        .s_val = REPEAT,
        .t_val = REPEAT,
      },
      .name = metadata.path.filename().string(),
    };
    metadata.asset = NewRef<Texture2D>(metadata.path, spec);

    return true;
  }

}  // namespace other