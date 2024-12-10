/**
 * \file editor/editor_asset_handler.cpp
 */
#include "editor/editor_asset_handler.hpp"

#include "core/logger.hpp"

#include "asset/asset_database.hpp"
#include "asset/asset_defines.hpp"

namespace other {

  AssetType EditorAssetHandler::GetAssetType(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      return GetAsset(handle)->GetAssetType();
    }
    return AssetType::BLANK_ASSET;
  }

  Ref<Asset> EditorAssetHandler::GetAsset(AssetHandle handle) {
    if (handle == 0) {
      OE_ERROR("Invalid asset handle : {}", handle);
      return nullptr;
    }

    if (!IsHandleValid(handle)) {
      OE_ERROR("Asset handle not valid : {}", handle);
      return nullptr;
    }

    AssetMetadata& metadata = GetMutableMetadata(handle);

    if (metadata.memory_asset) {
      OE_ASSERT(metadata.asset != nullptr, "Memory asset not found : {}", handle);
      return metadata.asset;
    }

    if (metadata.Unloaded()) {
      TryLoadAsset(metadata);
      if (!metadata.Loaded()) {
        OE_ERROR("Failed to load asset : {}", handle);
        return nullptr;
      }
    }
    OE_ASSERT(metadata.Loaded(), "Asset not loaded : {}", handle);
    return metadata.asset;
  }

  Ref<Asset> EditorAssetHandler::GetAsset(const AssetKey& key) {
    OE_ASSERT(AssetDatabase::HasKey(key), "Asset not found : {}", key.file_handle);

    AssetMetadata& metadata = AssetDatabase::Get(key);
    if (metadata.handle == 0) {
      TryLoadAsset(metadata);
    }

    return GetAsset(metadata.handle);
  }

  Ref<Asset> EditorAssetHandler::GetAsset(UUID file_handle, AssetType type) {
    AssetKey key = {
      .file_handle = file_handle,
      .type = type,
    };
    return GetAsset(key);
  }

  void EditorAssetHandler::AddMemOnly(const std::string_view virtual_filename, Ref<Asset>& asset) {
    OE_ASSERT(asset != nullptr, "Attempting to add null asset");

    OE_DEBUG("Adding memory asset : {} [{}]", virtual_filename, asset->handle);
    AssetDatabase::RegisterMemoryAsset(asset, virtual_filename);
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
    if (IsHandleValid(handle)) {
      return GetMetadata(handle).memory_asset;
    }
    return false;
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
    if (IsHandleValid(handle) && AssetDatabase::Contains(handle)) {
      Ref<Asset> asset = GetMetadata(handle).asset;
      OE_ASSERT(asset != nullptr, "Asset not found : {}", handle);
      return asset->CheckFlag(AssetFlag::ASSET_LOADED);
    }
    return false;
  }

  void EditorAssetHandler::Remove(AssetHandle handle) {
    if (IsHandleValid(handle)) {
      AssetDatabase::UnregisterAsset(handle);
    }
  }

  std::set<AssetHandle> EditorAssetHandler::GetAllOfType(AssetType type) {
    return AssetDatabase::GetAllOfType(type);
  }

}  // namespace other
