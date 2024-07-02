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
      virtual void AddMemOnly(Ref<Asset> asset) override {}
      virtual bool ReloadData(AssetHandle handle) override { return false; }

      virtual bool IsHandleValid(AssetHandle handle) override { return false; }
      virtual bool IsMemOnly(AssetHandle handle) override { return false; }
      virtual bool IsLoaded(AssetHandle handle) override { return false; }
      virtual bool IsValid(AssetHandle handle) override { return false; }
      virtual bool IsMissing(AssetHandle handle) override { return false; }

      virtual void Remove(AssetHandle handle) override {}

      virtual AssetSet GetAllOfType(AssetType type) override { return {}; }
      virtual AssetMap& GetAll() override { return TEMP; }

    private:
      AssetMap TEMP;
  }; 

} // namespace other

#endif // !OTHER_ENGINE_RUNTIME_ASSET_HANDLER_HPP
