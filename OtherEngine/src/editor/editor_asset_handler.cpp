/**
 * \file editor/editor_asset_handler.cpp
 */
#include "editor/editor_asset_handler.hpp"

#include "core/logger.hpp"
#include "asset/asset_loader.hpp"

namespace other {

  static AssetMetadata null_metadata;

  AssetType EditorAssetHandler::GetAssetType(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->GetAssetType();
    }
    return AssetType::BLANK_ASSET;
  }  

  Ref<Asset> EditorAssetHandler::GetAsset(AssetHandle handle) {
    Ref<Asset> asset = nullptr;

    if (IsMemOnly(handle)) {
      asset = memory_assets[handle];
    } else {
      auto& metadata = GetMetadata(handle);
      if (!metadata.IsValid()) {
        OE_ERROR("Asset [{}] not valid!");
        return nullptr;
      }

      Ref<Asset> asset = nullptr;
      if (!metadata.loaded) {
        metadata.loaded = AssetLoader::TryLoad(metadata , asset);
        LoadAsset(handle);
        // loaded_assets[handle] = asset;
      } else {
        OE_ASSERT(false , "Asset loading not implemented yet");
        /// asset = loaded_assets[handle];
      }
    }

    return asset;
  }

  void EditorAssetHandler::AddMemOnly(Ref<Asset> asset) {
    if (asset == nullptr) {
      OE_ERROR("Attempting to add null memory-only asset!");
      return;
    }

    AssetMetadata metadata;
    metadata.handle = asset->handle;
    metadata.loaded = true;
    metadata.type = asset->GetAssetType();
    metadata.memory_asset = true;
    registry[asset->handle] = metadata;
    memory_assets[asset->handle] = asset;
  }

  bool EditorAssetHandler::ReloadData(AssetHandle handle) {
    if (IsHandleValid(handle)) {
    }
    return false;
  }

  bool EditorAssetHandler::IsHandleValid(AssetHandle handle) {
    return assets.find(handle) != assets.end();
  }

  bool EditorAssetHandler::IsMemOnly(AssetHandle handle) {
    return memory_assets.find(handle) != memory_assets.end();
  }

  //bool EditorAssetHandler::IsAssetReadOnly(AssetHandle handle) {
  //  if (IsAssetHandleValid(handle)) {
  //    return GetAsset(handle)->CheckFlag(AssetFlag::READ_ONLY);
  //  }
  //  return false;
  //} 

  //bool EditorAssetHandler::IsAssetReadWrite(AssetHandle handle) {
  //  if (IsAssetHandleValid(handle)) {
  //    return GetAsset(handle)->CheckFlag(AssetFlag::READ_WRITE);
  //  }
  //  return false;
  //}

  bool EditorAssetHandler::IsMissing(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->CheckFlag(AssetFlag::MISSING);
    }
    return false;
  }

  // bool EditorAssetHandler::IsAssetDirty(AssetHandle handle) {
  //   if (IsAssetHandleValid(handle)) {
  //     return GetAsset(handle)->CheckFlag(AssetFlag::DIRTY);
  //   }
  //   return false;
  // }

  bool EditorAssetHandler::IsValid(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->IsValid();
    }
    return false;
  }
  
  bool EditorAssetHandler::IsLoaded(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      return assets[handle]->CheckFlag(AssetFlag::ASSET_LOADED);
    }
    return false;
  }

  void EditorAssetHandler::Remove(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      assets.erase(assets.find(handle));
    }
  }

  AssetSet EditorAssetHandler::GetAllOfType(AssetType type) {
    AssetSet result;
    for (auto& asset : assets) {
      if (asset.second->GetAssetType() == type) {
        result.insert(asset.first);
      }
    }
    return result;
  }

  AssetMap& EditorAssetHandler::GetAll() {
    return assets;
  }

  Ref<Asset> EditorAssetHandler::FindAsset(AssetHandle handle) {
    Ref<Asset> asset = nullptr;

    if (IsMemOnly(handle)) {
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
  
  AssetMetadata& EditorAssetHandler::GetMetadata(AssetHandle handle) {
    if (registry.Contains(handle)) {
      return registry[handle];
    }
    return null_metadata;
  }

  void EditorAssetHandler::LoadAsset(AssetHandle handle) {
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
