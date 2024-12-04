/**
 * \file asset/asset_database.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_DATABASE_HPP
#define OTHER_ENGINE_ASSET_DATABASE_HPP

#include "asset/asset_metadata.hpp"

namespace other {

  class AssetDatabase {
   public:
    static bool Contains(AssetHandle handle);
    static void RegisterAsset(const AssetMetadata& metadata);

   private:
    friend class AssetHandler;
    friend class EditorAssetHandler;
    friend class RuntimeAssetHandler;
    friend class AssetManager;
    friend class FileHandle;

    static void UnregisterAsset(const AssetMetadata& metadata);
    static void UnregisterAsset(const AssetHandle metadata);

    static AssetMetadata& Get(AssetHandle handle);
    static std::set<AssetHandle> GetAllOfType(AssetType type);

    static AssetMetadata& ProcessAsset(const AssetMetadata& metadata);
    static void AddAsset(const AssetMetadata& metadata);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_DATABASE_HPP