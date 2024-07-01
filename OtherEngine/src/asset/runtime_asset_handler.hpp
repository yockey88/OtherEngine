/**
 * \file asset/runtime_asset_handler.hpp
 **/
#ifndef OTHER_ENGINE_RUNTIME_ASSET_HANDLER_HPP
#define OTHER_ENGINE_RUNTIME_ASSET_HANDLER_HPP

#include "asset/asset_handler.hpp"

namespace other {


  class RuntimeAssetHandler : public AssetHandler {
    public:
      RuntimeAssetHandler() {}
      virtual ~RuntimeAssetHandler() override {}

      virtual AssetType GetAssetType(AssetHandle handle) override { return AssetType::INVALID_ASSET; }
      virtual Ref<Asset> GetAsset(AssetHandle handle) override { return nullptr; }
      virtual void AddMemoryOnlyAsset(Ref<Asset> asset) override {}
      virtual bool ReloadData(AssetHandle handle) override { return false; }

      virtual bool IsAssetHandleValid(AssetHandle handle) override { return false; }
      virtual bool IsMemoryAsset(AssetHandle handle) override { return false; }
      virtual bool IsAssetLoaded(AssetHandle handle) override { return false; }
      virtual bool IsAssetValid(AssetHandle handle) override { return false; }
      virtual bool IsAssetMissing(AssetHandle handle) override { return false; }

      virtual void RemoveAsset(AssetHandle handle) override {}

      virtual AssetSet GetAllAssetsOfType(AssetType type) override { return {}; }
      virtual AssetMap& GetAllAssets() override { return TEMP; }

    private:
      AssetMap TEMP;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RUNTIME_ASSET_HANDLER_HPP
