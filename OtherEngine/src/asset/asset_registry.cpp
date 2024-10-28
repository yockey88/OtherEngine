/**
 * \file asset\asset_registry.cpp
 **/
#include "asset\asset_registry.hpp"

namespace other {

  AssetMetadata& AssetRegistry::operator[](AssetHandle handle) {
    return assets[handle];
  }

  const AssetMetadata& AssetRegistry::operator[](AssetHandle handle) const {
    return assets.at(handle);
  }

  AssetMetadata& AssetRegistry::At(AssetHandle handle) {
    return assets.at(handle);
  }

  const AssetMetadata& AssetRegistry::At(AssetHandle handle) const {
    return assets.at(handle);
  }

  size_t AssetRegistry::Size() const {
    return assets.size();
  }

  bool AssetRegistry::Contains(AssetHandle handle) const {
    return assets.find(handle) != assets.end();
  }

  bool AssetRegistry::Empty() const {
    return assets.empty();
  }

  void AssetRegistry::Clear() {
    assets.clear();
  }

  AssetDataMap::iterator AssetRegistry::find(AssetHandle handle) {
    return assets.find(handle);
  }

  AssetDataMap::iterator AssetRegistry::begin() {
    return assets.begin();
  }

  AssetDataMap::iterator AssetRegistry::end() {
    return assets.end();
  }

  AssetDataMap::const_iterator AssetRegistry::cbegin() const {
    return assets.cbegin();
  }

  AssetDataMap::const_iterator AssetRegistry::cend() const {
    return assets.cend();
  }

}  // namespace other