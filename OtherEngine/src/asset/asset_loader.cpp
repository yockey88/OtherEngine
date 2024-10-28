/**
 * \file asset/asset_loader.cpp
 */
#include "asset/asset_loader.hpp"

#include "core/logger.hpp"

namespace other {

  std::unordered_map<AssetType, Scope<AssetSerializer>> AssetLoader::asset_loaders;

  void AssetLoader::Serialize(const Ref<Asset>& asset) {
    OE_ASSERT(false, "AssetLoader::Serialize unimplemented");
  }

  void AssetLoader::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) {
    OE_ASSERT(false, "AssetLoader::Serialize unimplemented");
  }

  Ref<Asset> AssetLoader::Load(const AssetMetadata& metadata) {
    OE_ASSERT(false, "AssetLoader::Load unimplemented");
    return nullptr;
  }

}  // namespace other
