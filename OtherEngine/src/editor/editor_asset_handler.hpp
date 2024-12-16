/**
 * \file editor\editor_asset_handler.hpp
 */
#ifndef OTHER_ENGINE_EDITOR_ASSET_HANDLER_HPP
#define OTHER_ENGINE_EDITOR_ASSET_HANDLER_HPP

#include "core/ref.hpp"

#include "asset/asset.hpp"
#include "asset/asset_defines.hpp"
#include "asset/asset_handler.hpp"
#include "asset/asset_registry.hpp"

namespace other {

  class EditorAssetHandler : public AssetHandler {
   public:
    EditorAssetHandler() {}
    virtual ~EditorAssetHandler() override {}

    virtual AssetType GetAssetType(AssetHandle handle) override;
    virtual Ref<Asset> GetAsset(AssetHandle handle) override;
    virtual Ref<Asset> GetAsset(UUID file_handle, AssetType type) override;
    virtual Ref<Asset> GetAsset(const AssetKey& key) override;

    virtual void AddMemOnly(const std::string_view virtual_filename, Ref<Asset>& asset) override;
    virtual bool ReloadData(AssetHandle handle) override;

    virtual bool IsHandleValid(AssetHandle handle) override;
    virtual bool IsMemOnly(AssetHandle handle) override;
    virtual bool IsLoaded(AssetHandle handle) override;
    virtual bool IsValid(AssetHandle handle) override;
    virtual bool IsMissing(AssetHandle handle) override;

    virtual void Remove(AssetKey key) override;
    virtual void Remove(AssetHandle handle) override;

    virtual std::set<AssetHandle> GetAllOfType(AssetType type) override;
    virtual std::set<AssetKey> GetAllKeysOfType(AssetType type) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_EDITOR_ASSET_HANDLER_HPP
