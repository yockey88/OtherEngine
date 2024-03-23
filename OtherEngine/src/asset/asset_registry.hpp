/**
 * \file asset/asset_registry.hpp
*/
#ifndef OTHER_ENGINE_ASSET_REGISTRY_HPP
#define OTHER_ENGINE_ASSET_REGISTRY_HPP

#include <unordered_map>

#include "asset/asset_types.hpp"
#include "asset/asset_metadata.hpp"

namespace other {

  using AssetDataMap = std::unordered_map<AssetHandle, AssetMetadata>;

  class AssetRegistry {
    public:
      AssetMetadata& operator[](AssetHandle handle) { return assets[handle]; }
      const AssetMetadata& operator[](AssetHandle handle) const { return assets.at(handle); }
      AssetMetadata& At(AssetHandle handle) { return assets.at(handle); }
      const AssetMetadata& At(AssetHandle handle) const { return assets.at(handle); }

      size_t Size() const { return assets.size(); }
      bool Contains(AssetHandle handle) const { return assets.find(handle) != assets.end(); }
      bool Empty() const { return assets.empty(); }
      void Clear() { assets.clear(); }

      auto begin() { return assets.begin(); }
      auto end() { return assets.end(); }
      auto cbegin() const { return assets.cbegin(); }
      auto cend() const { return assets.cend(); }

    private:
      AssetDataMap assets;
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_REGISTRY_HPP
