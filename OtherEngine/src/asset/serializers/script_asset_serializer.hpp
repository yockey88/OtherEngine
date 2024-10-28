/**
 * \file asset/script_asset_serializer.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_ASSET_SERIALIZER_HPP
#define OTHER_ENGINE_SCRIPT_ASSET_SERIALIZER_HPP

#include "core/ref.hpp"

#include "asset/asset.hpp"
#include "asset/asset_metadata.hpp"
#include "asset/asset_serializer.hpp"

namespace other {

  class ScriptFileAssetSerializer : public AssetSerializer {
   public:
    virtual ~ScriptFileAssetSerializer() {}

    virtual void Serialize(const Ref<Asset>& asset);
    virtual void Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset);
    virtual bool Load(const AssetMetadata& metadata, Ref<Asset>& asset);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCRIPT_ASSET_SERIALIZER_HPP