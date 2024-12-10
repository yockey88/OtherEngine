/**
 * \file asset\asset_registry.cpp
 **/
#include "asset\asset_registry.hpp"

#include <algorithm>

#include "core/file_handle.hpp"

#include "asset/asset.hpp"

namespace other {

  bool AssetMetadata::IsValid() const {
    return file_handle.Get() != 0 && type != AssetType::BLANK_ASSET;
  }

  bool AssetMetadata::Loaded() const {
    if (loaded) {
      OE_ASSERT(asset != nullptr, "Asset is loaded but asset is null : {}", handle);
      OE_ASSERT(asset->handle == handle, "Asset mismatch : {}", handle);
      OE_ASSERT(asset->CheckFlag(AssetFlag::ASSET_LOADED), "Asset not loaded : {}", handle);

      return true;
    } else {
      return false;
    }
  }

  bool AssetMetadata::Unloaded() const {
    if (!loaded) {
      OE_ASSERT(asset == nullptr, "Asset is not loaded but asset exists : {}", handle);
      return true;
    } else {
      return false;
    }
  }

  void AssetRegistry::AddAsset(const FileHandle* file) {
    OE_ASSERT(file != nullptr, "Invalid file handle");
    OE_ASSERT(file->handle.Get() != 0, "Invalid file handle");
    OE_ASSERT(file->Exists(), "File does not exist : {}", file->AbsolutePath().string());

    AssetKey key = {
      .file_handle = file->handle,
      .type = file->GetAssetType(),
    };

    if (HasKey(key)) {
      OE_WARN("[{}] asset already registered : {}\n > [{}]", file->GetAssetType(), file->AbsolutePath(), key);
      return;
    }

    OE_TRACE("{} ({})", key, Path(*file));

    AssetMetadata meta = {
      .handle = 0,
      .file_handle = file->handle,
      .type = file->GetAssetType(),
      .path = file->AbsolutePath(),
      .loaded = false,
      .memory_asset = false,
    };

    assets.insert({ key, meta });
    OE_ASSERT(assets.find(key) != assets.end(), "Failed to register [{}] asset : {}\n > [{}]", file->GetAssetType(), file->AbsolutePath(), key);
    OE_DEBUG("Registered [{}] asset : {}", file->GetAssetType(), meta);
  }

  void AssetRegistry::AddMemoryAsset(const AssetMetadata& metadata) {
    if (metadata.handle == 0) {
      OE_ERROR("Invalid asset handle");
      return;
    }

    AssetKey key = {
      .file_handle = metadata.file_handle,
      .type = metadata.type,
    };

    if (assets.find(key) != assets.end()) {
      OE_WARN("Asset with file handle [{}] already registered", metadata.file_handle);
      return;
    }
    OE_ASSERT(metadata.asset != nullptr, "Memory asset is null : {}", metadata.handle);

    OE_TRACE("Registering [{}] memory asset {}", metadata.asset->GetAssetType(), metadata.handle);
    assets.insert({ key, metadata });
    OE_ASSERT(assets.find(key) != assets.end(), "Failed to register memory asset : {}", metadata.handle);
  }

  bool AssetRegistry::Contains(AssetHandle handle) const {
    OE_TRACE("Checking for asset handle : {}", handle);
    return std::ranges::find_if(assets, [&](const auto& pair) -> bool { return pair.second.handle == handle; }) != assets.end();
  }

  bool AssetRegistry::HasKey(const AssetKey& key) const {
    OE_TRACE("Checking for asset key : {}", key);
    return assets.find(key) != assets.end();
  }

  AssetMetadata& AssetRegistry::GetMetadata(AssetHandle handle) {
    auto itr = std::ranges::find_if(assets, [&](const auto& pair) -> bool { return pair.second.handle == handle; });
    OE_ASSERT(itr != assets.end(), "Asset not found : {}", handle);
    return itr->second;
  }

  AssetMetadata& AssetRegistry::GetMetadata(const AssetKey& key) {
    return assets.at(key);
  }

  void AssetRegistry::RemoveAsset(AssetHandle handle) {
    auto itr = std::ranges::find_if(assets, [&](const auto& pair) -> bool { return pair.second.handle == handle; });
    if (itr == assets.end()) {
      OE_WARN("Asset not found : {}", handle);
      return;
    }

    assets.erase(itr);
  }

  void AssetRegistry::RemoveAsset(const AssetKey& key) {
    auto itr = assets.find(key);
    if (itr == assets.end()) {
      OE_WARN("Asset not found : {}", key.file_handle);
      return;
    }
    assets.erase(itr);
  }

  const std::unordered_map<AssetKey, AssetMetadata>& AssetRegistry::ReadAllAssets() const {
    return assets;
  }

}  // namespace other