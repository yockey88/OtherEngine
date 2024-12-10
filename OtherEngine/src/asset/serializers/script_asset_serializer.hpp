/**
 * \file asset/script_asset_serializer.hpp
 **/
#ifndef OTHER_ENGINE_SCRIPT_ASSET_SERIALIZER_HPP
#define OTHER_ENGINE_SCRIPT_ASSET_SERIALIZER_HPP

#include "core/ref.hpp"

#include "asset/asset.hpp"
#include "asset/asset_registry.hpp"
#include "asset/asset_serializer.hpp"

namespace other {

  class ScriptFileAssetSerializer : public AssetSerializer {
   public:
    virtual ~ScriptFileAssetSerializer() {}

    virtual void Serialize(const AssetMetadata& metadata) override;
    virtual bool Load(AssetMetadata& metadata) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SCRIPT_ASSET_SERIALIZER_HPP