/**
 * \file editor/editor_asset_handler.cpp
 */
#include "editor/editor_asset_handler.hpp"

#include "asset/asset_extensions.hpp"
#include "core/logger.hpp"
#include "core/filesystem.hpp"
#include "core/rand.hpp"
#include "asset/asset_loader.hpp"

namespace other {

  static AssetMetadata null_metadata;

  const AssetMetadata& EditorAssetHandler::GetMetadata(AssetHandle handle) {
    auto itr = registry.find(handle);
    if (itr == registry.end()) {
      return null_metadata;
    }

    return registry[handle];
  }
      
  const AssetMetadata& EditorAssetHandler::GetMetadata(const Path& path) {
    auto itr = std::ranges::find_if(registry , [&path](const auto& asset_pair) -> bool {
        return asset_pair.second.path == path;
    });
    if (itr != registry.end()) {
      return itr->second;
    }

    return null_metadata;
  }
  
  AssetMetadata& EditorAssetHandler::GetMutableMetadata(AssetHandle handle) {
    auto itr = registry.find(handle);
    if (itr == registry.end()) {
      return null_metadata;
    }

    return registry[handle];
  }
      
  AssetHandle EditorAssetHandler::ImportAsset(const Path& path) {
    if (!Filesystem::FileExists(path)) {
      return 0; 
    }

    if (auto& md = GetMetadata(path); md.IsValid()) {
      return md.handle;
    }

    AssetType type = GetAssetTypeFromPath(path);
    if (type == AssetType::INVALID_ASSET) {
      return 0;
    }

    AssetMetadata metadata;
    metadata.handle = Random::GenerateUUID(); 
    metadata.path = path;
    metadata.type = type;
    registry[metadata.handle] = metadata;

    return metadata.handle;
  }
      
  AssetHandle EditorAssetHandler::GetAssetHandleFromFilePath(const Path& filepath) {
    auto itr = std::ranges::find_if(registry , [&filepath](const auto& asset_pair) -> bool {
      return asset_pair.second.path == filepath; 
    });

    if (itr == registry.end()) {
      return 0;
    }

    return itr->first;
  }
      
  AssetType EditorAssetHandler::GetAssetTypeFromExtension(const std::string& extension) {
    auto itr = asset_extensions.find(FNV(extension)); 
    if (itr == asset_extensions.end()) {
      return AssetType::INVALID_ASSET;
    }

    return itr->second;
  }

  AssetType EditorAssetHandler::GetAssetTypeFromPath(const Path& path) {
    return GetAssetTypeFromExtension(path.extension().string());
  }

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
      auto& metadata = GetMutableMetadata(handle);
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

  void EditorAssetHandler::AddMemOnly(Ref<Asset>& asset) {
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
    return registry.find(handle) != registry.end() && 
           (assets.find(handle) != assets.end() || 
            memory_assets.find(handle) != memory_assets.end()); 
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

  void EditorAssetHandler::LoadAsset(AssetHandle handle) {
    auto& metadata = GetMutableMetadata(handle);
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
