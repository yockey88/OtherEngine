/**
 * \file asset/asset_loader.cpp
*/
#include "asset/asset_loader.hpp"

#include "core/logger.hpp"

namespace other {

  std::unordered_map<AssetType , Scope<AssetSerializer>> AssetLoader::asset_loaders;

  void AssetLoader::Serialize(const Ref<Asset>& asset) {
    OE_ASSERT(false , "AssetLoader::Serialize unimplemented");
  }

  void AssetLoader::Serialize(const AssetMetadata& metadata , const Ref<Asset>& asset) {
    OE_ASSERT(false , "AssetLoader::Serialize unimplemented");
  }

  bool AssetLoader::Load(const AssetMetadata& metadata , Ref<Asset>& asset) {
    OE_ASSERT(false , "AssetLoader::Load unimplemented");
    return false;
  }

  bool AssetLoader::TryLoad(const AssetMetadata& metadata , Ref<Asset>& asset) {
    OE_ASSERT(false , "AssetLoader::TryLoad unimplemented");
    return false;
  } 



} // namespace other
