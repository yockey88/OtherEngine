/**
 * \file asset/asset_registry.hpp
 */
#ifndef OTHER_ENGINE_ASSET_REGISTRY_HPP
#define OTHER_ENGINE_ASSET_REGISTRY_HPP

#include <map>

#include "asset/asset_metadata.hpp"
#include "asset/asset_types.hpp"

namespace other {

  using AssetDataMap = std::map<AssetHandle, AssetMetadata>;

  class AssetRegistry {
   public:
    AssetMetadata& operator[](AssetHandle handle);
    const AssetMetadata& operator[](AssetHandle handle) const;
    AssetMetadata& At(AssetHandle handle);
    const AssetMetadata& At(AssetHandle handle) const;

    size_t Size() const;
    bool Contains(AssetHandle handle) const;
    bool Empty() const;
    void Clear();

    AssetDataMap::iterator find(AssetHandle handle);
    AssetDataMap::iterator begin();
    AssetDataMap::iterator end();
    AssetDataMap::const_iterator cbegin() const;
    AssetDataMap::const_iterator cend() const;

   private:
    AssetDataMap assets;

    friend class AssetDatabase;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_REGISTRY_HPP
