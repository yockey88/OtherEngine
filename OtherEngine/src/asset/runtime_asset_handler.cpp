/**
 * \file asset/runtime_asset_handler.cpp
 **/
#include "asset/runtime_asset_handler.hpp"

#include "core/logger.hpp"

#include "asset/asset_loader.hpp"

namespace other {

  AssetType RuntimeAssetHandler::GetAssetType(AssetHandle handle) { 
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->GetAssetType();
    }
    return AssetType::BLANK_ASSET;
  }
  
  Ref<Asset> RuntimeAssetHandler::GetAsset(AssetHandle handle) { 
    Ref<Asset> asset = nullptr;

    if (IsMemOnly(handle)) {
      asset = memory_assets[handle];
    } else {
      auto& metadata = GetMetadata(handle);
      if (!metadata.IsValid()) {
        // OE_ERROR("Asset [{}] not valid!" , handle);
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
  
  void RuntimeAssetHandler::AddMemOnly(Ref<Asset> asset) {
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
  
  bool RuntimeAssetHandler::ReloadData(AssetHandle handle) { 
    if (IsHandleValid(handle)) {
    }
    return false;
  }
  
  bool RuntimeAssetHandler::IsHandleValid(AssetHandle handle) { 
    return registry.find(handle) != registry.end() && 
           (assets.find(handle) != assets.end() || 
            memory_assets.find(handle) != memory_assets.end()); 
  }
  
  bool RuntimeAssetHandler::IsMemOnly(AssetHandle handle) { 
    return memory_assets.find(handle) != memory_assets.end();
  }
  
  bool RuntimeAssetHandler::IsLoaded(AssetHandle handle) { 
    if (IsHandleValid(handle)) {
      return assets[handle]->CheckFlag(AssetFlag::ASSET_LOADED);
    }
    return false;
  }
  
  bool RuntimeAssetHandler::IsValid(AssetHandle handle) { 
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->IsValid();
    }
    return false;
  }
  
  bool RuntimeAssetHandler::IsMissing(AssetHandle handle) { 
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->CheckFlag(AssetFlag::MISSING);
    }
    return false;
  }
  
  void RuntimeAssetHandler::Remove(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      assets.erase(assets.find(handle));
    }
  }
  
  AssetSet RuntimeAssetHandler::GetAllOfType(AssetType type) { 
    AssetSet result;
    for (auto& asset : assets) {
      if (asset.second->GetAssetType() == type) {
        result.insert(asset.first);
      }
    }
    return result;
  }
  
  AssetMap& RuntimeAssetHandler::GetAll() { 
    return assets;
  }
      
  Ref<Asset> RuntimeAssetHandler::FindAsset(AssetHandle handle) {
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

  static AssetMetadata null_metadata;
  AssetMetadata& RuntimeAssetHandler::GetMetadata(AssetHandle handle) {
    if (registry.Contains(handle)) {
      return registry[handle];
    }
    return null_metadata;
  }

  void RuntimeAssetHandler::LoadAsset(AssetHandle handle) {
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
