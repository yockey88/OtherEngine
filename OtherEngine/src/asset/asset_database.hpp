/**
 * \file asset/asset_database.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_DATABASE_HPP
#define OTHER_ENGINE_ASSET_DATABASE_HPP

#include <set>

#include "asset/asset_defines.hpp"
#include "asset/asset_registry.hpp"

namespace other {

  class Asset;

  class AssetDatabase {
   public:
    static bool Contains(AssetHandle handle);
    static bool HasKey(AssetKey key);

    static void RegisterAsset(const FileHandle* file);
    static void RegisterMemoryAsset(Ref<Asset>& asset, const std::string_view virtual_filename);
    static void RegisterAssetHandle(const AssetKey& key, AssetHandle handle);
    static void RegisterAssetHandle(UUID file_handle, AssetType type, AssetHandle handle);
    static void RegisterLoadedAsset(Asset* asset);

    static AssetHandle GetHandle(const AssetKey& key);
    static AssetMetadata& Get(const AssetKey& key);
    static AssetMetadata& Get(AssetHandle handle);

   private:
    friend class AssetHandler;
    friend class EditorAssetHandler;
    friend class RuntimeAssetHandler;
    friend class AssetManager;
    friend class FileHandle;

    static void UnregisterAsset(const AssetKey& key);
    static void UnregisterAsset(const AssetMetadata& metadata);
    static void UnregisterAsset(const AssetHandle metadata);

    static std::set<AssetHandle> GetAllOfType(AssetType type);
    static std::set<AssetKey> GetAllKeysOfType(AssetType type);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_DATABASE_HPP