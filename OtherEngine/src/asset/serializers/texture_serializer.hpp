/**
 * \file asset/serializers/texture_serializer.hpp
 **/
#ifndef OTHER_ENGINE_TEXTURE_SERIALIZER_HPP
#define OTHER_ENGINE_TEXTURE_SERIALIZER_HPP

#include "asset/asset_serializer.hpp"

namespace other {

  class TextureSerializer : public AssetSerializer {
   public:
    virtual ~TextureSerializer() override {}

    virtual void Serialize(const AssetMetadata& metadata) override;
    virtual bool Load(AssetMetadata& metadata) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_TEXTURE_SERIALIZER_HPP