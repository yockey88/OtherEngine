/**
 * \file asset/asset_serializer.hpp
*/
#ifndef OTHER_ENGINE_ASSET_SERIALIZER_HPP
#define OTHER_ENGINE_ASSET_SERIALIZER_HPP

#include "asset/asset.hpp"
#include "asset/asset_metadata.hpp"

namespace other {

  class AssetSerializer {
    public:
      virtual ~AssetSerializer() {}

      virtual void Serialize(const Ref<Asset>& asset) = 0;
      virtual void Serialize(const AssetMetadata& metadata , const Ref<Asset>& asset) = 0;
      virtual bool Load(const AssetMetadata& metadata , Ref<Asset>& asset) = 0;
  };

  class TextureSerializer : public AssetSerializer {
    public:
      void Serialize(const AssetMetadata& metadata , const Ref<Asset>& asset) override {}
      bool Load(const AssetMetadata& metadata , Ref<Asset>& asset) override { return false; }
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_SERIALIZER_HPP
