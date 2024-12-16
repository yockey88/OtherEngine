/**
 * \file asset/asset_handler.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_HANDLER_HPP
#define OTHER_ENGINE_ASSET_HANDLER_HPP

#include "core/ref.hpp"
#include "core/ref_counted.hpp"

#include "asset/asset.hpp"
#include "asset/asset_defines.hpp"
#include "asset/asset_registry.hpp"

namespace other {

  class AssetHandler;

  class AssetHandler : public RefCounted {
   public:
    AssetHandler() {}
    virtual ~AssetHandler() {}

    const AssetMetadata& GetMetadata(AssetHandle handle);
    const AssetMetadata& GetMetadata(const Path& path);
    const AssetMetadata& GetMetadata(const AssetKey& key);
    AssetMetadata& GetMutableMetadata(AssetHandle handle);

    AssetHandle ImportAsset(const Path& path);
    AssetHandle GetAssetHandleFromFilePath(const Path& filepath);

    AssetType GetAssetTypeFromExtension(const std::string& extension);
    AssetType GetAssetTypeFromPath(const Path& path);

    virtual AssetType GetAssetType(AssetHandle assetHandle) = 0;
    virtual Ref<Asset> GetAsset(AssetHandle assetHandle) = 0;
    virtual Ref<Asset> GetAsset(UUID file_handle, AssetType type) = 0;
    virtual Ref<Asset> GetAsset(const AssetKey& key) = 0;

    virtual void AddMemOnly(const std::string_view virtual_filename, Ref<Asset>& asset) = 0;
    virtual bool ReloadData(AssetHandle assetHandle) = 0;

    // the asset handle is valid (this says nothing about the asset itself)
    virtual bool IsHandleValid(AssetHandle assetHandle) = 0;
    // asset exists in memory only, there is no backing file
    virtual bool IsMemOnly(AssetHandle handle) = 0;
    // asset has been loaded from file (it could still be invalid)
    virtual bool IsLoaded(AssetHandle handle) = 0;
    // asset file was loaded, but is invalid for some reason (e.g. corrupt file)
    virtual bool IsValid(AssetHandle handle) = 0;
    // asset file is missing
    virtual bool IsMissing(AssetHandle handle) = 0;

    virtual void Remove(AssetKey key) = 0;
    virtual void Remove(AssetHandle handle) = 0;

    virtual std::set<AssetHandle> GetAllOfType(AssetType type) = 0;
    virtual std::set<AssetKey> GetAllKeysOfType(AssetType type) = 0;

   protected:
    void TryLoadAsset(AssetMetadata& metadata);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_HANDLER_HPP
