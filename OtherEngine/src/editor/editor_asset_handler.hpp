/**
 * \file editor\editor_asset_handler.hpp
*/
#ifndef OTHER_ENGINE_EDITOR_ASSET_HANDLER_HPP
#define OTHER_ENGINE_EDITOR_ASSET_HANDLER_HPP

#include "core/ref.hpp"

#include "asset/asset_types.hpp"
#include "asset/asset.hpp"
#include "asset/asset_registry.hpp"
#include "asset/asset_handler.hpp"

namespace other {

  class EditorAssetHandler : public AssetHandler {
    public:
      EditorAssetHandler() {}
      virtual ~EditorAssetHandler() override {}

      virtual AssetType GetAssetType(AssetHandle handle) override;
      virtual Ref<Asset> GetAsset(AssetHandle handle) override;
      virtual void AddMemoryOnlyAsset(Ref<Asset> asset) override;
      virtual bool ReloadData(AssetHandle handle) override;

      virtual bool IsAssetHandleValid(AssetHandle handle) override;
      virtual bool IsMemoryAsset(AssetHandle handle) override;
      virtual bool IsAssetLoaded(AssetHandle handle) override;
      virtual bool IsAssetValid(AssetHandle handle) override;
      virtual bool IsAssetMissing(AssetHandle handle) override;

      virtual void RemoveAsset(AssetHandle handle) override;

      virtual AssetSet GetAllAssetsOfType(AssetType type) override;
      virtual AssetMap& GetAllAssets() override;

    private:
      AssetMap assets;
      AssetMap memory_assets;

      AssetRegistry registry;

      Ref<Asset> FindAsset(AssetHandle handle);

      AssetMetadata& GetMetadata(AssetHandle handle);

      void LoadAsset(AssetHandle handle);
  };

} // namespace other

#endif // !OTHER_ENGINE_EDITOR_ASSET_HANDLER_HPP
