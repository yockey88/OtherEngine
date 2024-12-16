/**
 * \file asset/asset_handler.cpp
 **/
#include "asset/asset_handler.hpp"

#include "core/defines.hpp"
#include "core/filesystem.hpp"

#include "asset/asset.hpp"
#include "asset/asset_database.hpp"
#include "asset/asset_defines.hpp"
#include "asset/asset_loader.hpp"

namespace other {

  static AssetMetadata null_metadata;

  const AssetMetadata& AssetHandler::GetMetadata(AssetHandle handle) {
    if (AssetDatabase::Contains(handle)) {
      return AssetDatabase::Get(handle);
    } else {
      return null_metadata;
    }
  }

  const AssetMetadata& AssetHandler::GetMetadata(const Path& path) {
    Ref<FileHandle> file = Filesystem::GetFile(path);
    if (file == nullptr) {
      return null_metadata;
    }

    AssetHandle handle = file->handle;
    /// if it exists, is and asset and is in the database
    if (!file->Exists() || !AssetDatabase::Contains(handle)) {
      return null_metadata;
    }

    return AssetDatabase::Get(handle);
  }

  const AssetMetadata& AssetHandler::GetMetadata(const AssetKey& key) {
    if (AssetDatabase::HasKey(key)) {
      return AssetDatabase::Get(key);
    } else {
      return null_metadata;
    }
  }

  AssetMetadata& AssetHandler::GetMutableMetadata(AssetHandle handle) {
    if (AssetDatabase::Contains(handle)) {
      return AssetDatabase::Get(handle);
    } else {
      return null_metadata;
    }
  }

  AssetHandle AssetHandler::ImportAsset(const Path& path) {
    // Ref<FileHandle> file = Filesystem::GetFile(path);
    // if (file == nullptr || !file->Exists() || !file->IsAsset()) {
    //   return 0;
    // }

    // AssetHandle handle = file->handle;
    // if (AssetDatabase::Contains(handle)) {
    //   return handle;
    // }

    // AssetType type = GetAssetTypeFromPath(path);
    // if (type == AssetType::INVALID_ASSET) {
    //   return 0;
    // }

    // AssetDatabase::RegisterAsset({
    //   .handle = handle,
    //   .type = type,
    //   .path = path,
    //   .loaded = false,
    // });
    // OE_ASSERT(AssetDatabase::Contains(handle), "Failed to register asset : {}", handle);

    // return handle;
    return 0;
  }

  AssetHandle AssetHandler::GetAssetHandleFromFilePath(const Path& filepath) {
    Ref<FileHandle> file = Filesystem::GetFile(filepath);
    if (file == nullptr || !file->Exists()) {
      return 0;
    }

    if (AssetDatabase::Contains(file->handle)) {
      return file->handle;
    }

    return 0;
  }

  AssetType AssetHandler::GetAssetTypeFromExtension(const std::string& extension) {
    auto itr = asset_extensions.find(FNV(extension));
    if (itr == asset_extensions.end()) {
      return AssetType::INVALID_ASSET;
    }

    return itr->second;
  }

  AssetType AssetHandler::GetAssetTypeFromPath(const Path& path) {
    return GetAssetTypeFromExtension(path.extension().string());
  }

  void AssetHandler::TryLoadAsset(AssetMetadata& metadata) {
    OE_ASSERT(!metadata.loaded && !metadata.memory_asset, "Asset already loaded : {}", metadata.handle);

    OE_ASSERT(metadata.asset == nullptr, "Asset handle is 0 but asset exists : {}", metadata.path.empty() ? "<no path>" : metadata.path);
    auto asset = AssetLoader::Load(metadata);
    if (asset == nullptr) {
      OE_ERROR("Failed to load asset : {}", metadata.path.empty() ? "<no path>" : metadata.path);
      return;
    }
    OE_ASSERT(asset->handle != 0, "Asset handle is 0 : {}", metadata.path.empty() ? "<no path>" : metadata.path);
    metadata.handle = asset->handle;
    metadata.asset = asset.Raw();
    asset->SetFlag(AssetFlag::ASSET_LOADED, true);
    metadata.loaded = true;
  }

}  // namespace other
