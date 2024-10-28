/**
 * \file editor/editor_asset_handler.cpp
 */
#include "editor/editor_asset_handler.hpp"

#include "core/filesystem.hpp"
#include "core/logger.hpp"
#include "core/rand.hpp"

#include "asset/asset_database.hpp"
#include "asset/asset_extensions.hpp"
#include "asset/asset_loader.hpp"
#include "asset/asset_types.hpp"

namespace other {

  static AssetMetadata null_metadata;

  const AssetMetadata& EditorAssetHandler::GetMetadata(AssetHandle handle) {
    if (AssetDatabase::Contains(handle)) {
      return AssetDatabase::Get(handle);
    } else {
      return null_metadata;
    }
  }

  const AssetMetadata& EditorAssetHandler::GetMetadata(const Path& path) {
    Ref<FileHandle> file = Filesystem::GetFile(path);
    if (file == nullptr) {
      return null_metadata;
    }

    AssetHandle handle = file->handle;
    /// if it exists, is and asset and is in the database
    if (!file->Exists() || !file->IsAsset() || !AssetDatabase::Contains(handle)) {
      return null_metadata;
    }

    return AssetDatabase::Get(handle);
  }

  AssetMetadata& EditorAssetHandler::GetMutableMetadata(AssetHandle handle) {
    if (AssetDatabase::Contains(handle)) {
      return AssetDatabase::Get(handle);
    } else {
      return null_metadata;
    }
  }

  AssetHandle EditorAssetHandler::ImportAsset(const Path& path) {
    Ref<FileHandle> file = Filesystem::GetFile(path);
    if (file == nullptr || !file->Exists() || !file->IsAsset()) {
      return 0;
    }

    AssetHandle handle = file->handle;
    if (AssetDatabase::Contains(handle)) {
      return handle;
    }

    AssetType type = GetAssetTypeFromPath(path);
    if (type == AssetType::INVALID_ASSET) {
      return 0;
    }

    AssetDatabase::RegisterAsset({
      .handle = handle,
      .type = type,
      .path = path,
      .loaded = false,
    });
    OE_ASSERT(AssetDatabase::Contains(handle), "Failed to register asset : {}", handle);

    return handle;
  }

  AssetHandle EditorAssetHandler::GetAssetHandleFromFilePath(const Path& filepath) {
    Ref<FileHandle> file = Filesystem::GetFile(filepath);
    if (file == nullptr || !file->Exists() || !file->IsAsset()) {
      return 0;
    }

    if (AssetDatabase::Contains(file->handle)) {
      return file->handle;
    }

    return 0;
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
        asset = AssetLoader::Load(metadata);
        metadata.loaded = asset != nullptr;
        LoadAsset(handle);
        // loaded_assets[handle] = asset;
      } else {
        OE_ASSERT(false, "Asset loading not implemented yet");
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

    AssetDatabase::RegisterAsset({
      .handle = asset->handle,
      .type = asset->GetAssetType(),
      .path = Path(""),
      .loaded = true,
      .memory_asset = true,
    });
    OE_ASSERT(AssetDatabase::Contains(asset->handle), "Failed to register memory-only asset : {}", asset->handle);
    memory_assets[asset->handle] = asset;
  }

  bool EditorAssetHandler::ReloadData(AssetHandle handle) {
    if (IsHandleValid(handle)) {
    }
    return false;
  }

  bool EditorAssetHandler::IsHandleValid(AssetHandle handle) {
    return AssetDatabase::Contains(handle);
  }

  bool EditorAssetHandler::IsMemOnly(AssetHandle handle) {
    return memory_assets.find(handle) != memory_assets.end();
  }

  // bool EditorAssetHandler::IsAssetReadOnly(AssetHandle handle) {
  //   if (IsAssetHandleValid(handle)) {
  //     return GetAsset(handle)->CheckFlag(AssetFlag::READ_ONLY);
  //   }
  //   return false;
  // }

  // bool EditorAssetHandler::IsAssetReadWrite(AssetHandle handle) {
  //   if (IsAssetHandleValid(handle)) {
  //     return GetAsset(handle)->CheckFlag(AssetFlag::READ_WRITE);
  //   }
  //   return false;
  // }

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
      OE_ERROR("Asset not found: {0}", handle);
      return nullptr;
    }

    if (!metadata.loaded) {
      LoadAsset(handle);
    }

    asset = assets[handle];

    return asset;
  }

  void EditorAssetHandler::LoadAsset(AssetHandle handle) {
    // auto& metadata = GetMutableMetadata(handle);
    // if (metadata.IsValid()) {
    //   metadata.loaded = AssetLoader::Load(metadata, assets[handle]);
    //   if (!metadata.loaded) {
    //     OE_ERROR("Failed to load asset: {0}", handle);
    //   } else {
    //     assets[handle]->SetFlag(AssetFlag::ASSET_LOADED);
    //   }
    // }
    OE_ASSERT(false, "Asset loading not implemented yet");
  }

}  // namespace other
