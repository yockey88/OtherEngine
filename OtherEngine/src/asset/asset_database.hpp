/**
 * \file asset/asset_database.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_DATABASE_HPP
#define OTHER_ENGINE_ASSET_DATABASE_HPP

#include "asset/asset_metadata.hpp"

namespace other {

  class AssetDatabase {
   public:
    static void RegisterAsset(const AssetMetadata& metadata);
    static void UnregisterAsset(const AssetMetadata& metadata);
    static void UnregisterAsset(const AssetHandle metadata);
    static bool Contains(AssetHandle handle);

    static AssetMetadata& Get(AssetHandle handle);

   private:
    static AssetMetadata& ProcessAsset(const AssetMetadata& metadata);
    static void AddAsset(const AssetMetadata& metadata);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_DATABASE_HPP