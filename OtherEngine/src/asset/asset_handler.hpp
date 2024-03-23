/**
 * \file asset\asset_handler.hpp
*/
#ifndef OTHER_ENGINE_ASSET_HANDLER_HPP
#define OTHER_ENGINE_ASSET_HANDLER_HPP

#include <unordered_map>
#include <unordered_set>

#include "asset/asset_types.hpp"
#include "asset/asset.hpp"
#include "asset/asset_registry.hpp"

namespace other {

  using AssetSet = std::unordered_set<AssetHandle>;
  using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

  class AssetHandler {
    public:
      AssetHandler() {}
      ~AssetHandler() {}

      AssetType GetType(AssetHandle handle);
      Ref<Asset> GetAsset(AssetHandle handle);

      void AddMemoryOnlyAsset(Ref<Asset> asset);
      bool ReloadData(AssetHandle handle);

      bool IsAssetHandleValid(AssetHandle handle);
      bool IsMemoryAsset(AssetHandle handle);
      bool IsAssetReadOnly(AssetHandle handle);
      bool IsAssetReadWrite(AssetHandle handle);
      bool IsAssetMissing(AssetHandle handle);
      bool IsAssetDirty(AssetHandle handle);
      bool IsAssetValid(AssetHandle handle);
      bool IsAssetLoaded(AssetHandle handle);

      void RemoveAsset(AssetHandle handle);

      AssetSet GetAllAssetsOfType(AssetType type);
      AssetMap& GetAllAssets();

    private:
      AssetMap assets;
      AssetMap memory_assets;

      AssetRegistry registry;

      Ref<Asset> FindAsset(AssetHandle handle);

      AssetMetadata& GetMetadata(AssetHandle handle);

      void LoadAsset(AssetHandle handle);
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_HANDLER_HPP
