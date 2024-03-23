/**
 * \file asset/asset_loader.cpp
*/
#include "asset/asset_loader.hpp"

namespace other {

  std::unordered_map<AssetType , Scope<AssetSerializer>> AssetLoader::asset_loaders;

  void AssetLoader::Initialize(const ConfigTable& config) {

  }

  void AssetLoader::Serialize(const Ref<Asset>& asset) {
    if (asset_loaders.find(asset->GetAssetType()) != asset_loaders.end()) {
      asset_loaders[asset->GetAssetType()]->Serialize(asset);
    }  
  }

  void AssetLoader::Serialize(const AssetMetadata& metadata , const Ref<Asset>& asset) {
    if (asset_loaders.find(metadata.type) != asset_loaders.end()) {
      asset_loaders[metadata.type]->Serialize(metadata , asset);
    }
  }

  bool AssetLoader::Load(const AssetMetadata& metadata , Ref<Asset>& asset) {
    if (asset_loaders.find(metadata.type) != asset_loaders.end()) {
      return asset_loaders[metadata.type]->Load(metadata , asset);
    }
    return false;
  }



} // namespace other
