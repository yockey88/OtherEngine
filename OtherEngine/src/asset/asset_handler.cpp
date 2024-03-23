/**
 * \file asset/asset_handler.cpp
 */
#include "asset/asset_handler.hpp"

#include "core/logger.hpp"
#include "asset/asset_loader.hpp"

namespace other {

  static AssetMetadata null_metadata;

  AssetType AssetHandler::GetType(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return GetAsset(handle)->GetAssetType();
    }
    return AssetType::BLANK_ASSET;
  }  

  Ref<Asset> AssetHandler::GetAsset(AssetHandle handle) {
    Ref<Asset> asset = nullptr;

    if (IsMemoryAsset(handle)) {
      asset = memory_assets[handle];
    } else {
      // auto& metadata = GetMetadata(handle);
      // if (metadata.IsValid()) {
      //   if (!metadata.loaded) {
      //     LoadAsset(handle);
      //   }

      // } else {
      //   OE_ERROR("Asset not found: {0}" , handle);
      // }
    }

    return asset;
  }

  void AssetHandler::AddMemoryOnlyAsset(Ref<Asset> asset) {
    if (asset) {
      memory_assets[asset->asset_handle] = asset;
    }
  }

  bool AssetHandler::ReloadData(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
    }
    return false;
  }

  bool AssetHandler::IsAssetHandleValid(AssetHandle handle) {
    return assets.find(handle) != assets.end();
  }

  bool AssetHandler::IsMemoryAsset(AssetHandle handle) {
    return memory_assets.find(handle) != memory_assets.end();
  }

  bool AssetHandler::IsAssetReadOnly(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return GetAsset(handle)->CheckFlag(AssetFlag::READ_ONLY);
    }
    return false;
  } 

  bool AssetHandler::IsAssetReadWrite(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return GetAsset(handle)->CheckFlag(AssetFlag::READ_WRITE);
    }
    return false;
  }

  bool AssetHandler::IsAssetMissing(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return GetAsset(handle)->CheckFlag(AssetFlag::MISSING);
    }
    return false;
  }

  bool AssetHandler::IsAssetDirty(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return GetAsset(handle)->CheckFlag(AssetFlag::DIRTY);
    }
    return false;
  }

  bool AssetHandler::IsAssetValid(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return GetAsset(handle)->IsValid();
    }
    return false;
  }
  
  bool AssetHandler::IsAssetLoaded(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      return assets[handle]->CheckFlag(AssetFlag::ASSET_LOADED);
    }
    return false;
  }

  void AssetHandler::RemoveAsset(AssetHandle handle) {
    if (IsAssetHandleValid(handle)) {
      assets.erase(handle);
    }
  }

  AssetSet AssetHandler::GetAllAssetsOfType(AssetType type) {
    AssetSet result;
    for (auto& asset : assets) {
      if (asset.second->GetAssetType() == type) {
        result.insert(asset.first);
      }
    }
    return result;
  }

  AssetMap& AssetHandler::GetAllAssets() {
    return assets;
  }

  Ref<Asset> AssetHandler::FindAsset(AssetHandle handle) {
    Ref<Asset> asset = nullptr;

    if (IsMemoryAsset(handle)) {
      asset = memory_assets[handle];
      return asset;
    } 
    
    auto& metadata = GetMetadata(handle);
    if (!metadata.IsValid()) {
      OE_ERROR("Asset not found: {0}" , handle);
      return nullptr;
    }

    if (!metadata.loaded) {
      LoadAsset(handle);
    } 

    asset = assets[handle];

    return asset;
  }
  
  AssetMetadata& AssetHandler::GetMetadata(AssetHandle handle) {
    if (registry.Contains(handle)) {
      return registry[handle];
    }
    return null_metadata;
  }

  void AssetHandler::LoadAsset(AssetHandle handle) {
    auto& metadata = GetMetadata(handle);
    if (metadata.IsValid()) {
      metadata.loaded = AssetLoader::Load(metadata , assets[handle]);
      if (!metadata.loaded) {
        OE_ERROR("Failed to load asset: {0}" , handle);
      } else {
        assets[handle]->SetFlag(AssetFlag::ASSET_LOADED);
      }
    }
  }

} // namespace other
