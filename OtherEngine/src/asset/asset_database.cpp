/**
 * \file asset/asset_database.cpp
 **/
#include "asset/asset_database.hpp"

#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "asset/asset.hpp"
#include "asset/asset_defines.hpp"
#include "asset/asset_registry.hpp"

namespace other {
  namespace {

    static AssetRegistry asset_registry;

  }  // anonymous namespace

  bool AssetDatabase::Contains(AssetHandle handle) {
    return asset_registry.Contains(handle);
  }

  bool AssetDatabase::HasKey(AssetKey key) {
    return asset_registry.HasKey(key);
  }

  void AssetDatabase::RegisterAsset(const FileHandle* file) {
    OE_ASSERT(file != nullptr, "Invalid file handle");
    if (!file->Exists()) {
      OE_ERROR("File does not exist : {}", file->AbsolutePath().string());
      return;
    }

    asset_registry.AddAsset(file);
  }

  void AssetDatabase::RegisterMemoryAsset(Ref<Asset>& asset, const std::string_view virtual_filename) {
    OE_ASSERT(asset != nullptr, "Invalid asset");

    /// @TODO: grab virtual dir from asset type and attach appropriate extension
    std::string drive = "memory://";
    std::string ext = ".asset";
    std::string vpath = fmtstr("{}{}{}", drive, virtual_filename, ext);

    Ref<FileHandle> file = Filesystem::CreateMemoryFile(drive, virtual_filename, ext);
    OE_ASSERT(file != nullptr, "Failed to create memory file : {}", vpath);
    OE_ASSERT(file->Exists(), "Memory file does not exist : {}", vpath);
    OE_DEBUG("Virtual file created : {}", vpath);

    AssetMetadata md = {
      .handle = asset->handle,
      .file_handle = FNV(vpath),
      .type = asset->GetAssetType(),
      .asset = asset,
      .path = vpath,
      .loaded = true,
      .memory_asset = true,
    };
    OE_DEBUG("Registering [{}] memory asset : {} [{}]", md.type, vpath, md.handle);
    asset_registry.AddMemoryAsset(md);
  }

  void AssetDatabase::RegisterAssetHandle(const AssetKey& key, AssetHandle handle) {
    if (!HasKey(key)) {
      OE_ERROR("Asset key not found : {}", key.file_handle);
      return;
    }

    AssetMetadata& md = asset_registry.GetMetadata(key);
    md.handle = handle;
  }

  void AssetDatabase::RegisterAssetHandle(UUID file_handle, AssetType type, AssetHandle handle) {
    AssetKey key = {
      .file_handle = file_handle,
      .type = type,
    };

    RegisterAssetHandle(key, handle);
  }

  void AssetDatabase::RegisterLoadedAsset(Asset* asset) {
    OE_ASSERT(asset != nullptr, "Invalid asset");
    OE_ASSERT(asset->handle != 0, "Invalid asset handle");

    auto itr = std::ranges::find_if(asset_registry.ReadAllAssets(), [&](const auto& pair) -> bool { return pair.second.handle == asset->handle; });
    if (asset->CheckFlag(AssetFlag::ASSET_LOADED)) {
      OE_WARN("Asset already loaded : {}", asset->handle);
      OE_ASSERT(itr != asset_registry.ReadAllAssets().end(), "Asset not found : {}", asset->handle);
      OE_ASSERT(itr->second.asset != nullptr && itr->second.handle == asset->handle, "Asset mismatch : {}", asset->handle);
      return;
    }

    AssetMetadata& md = asset_registry.GetMetadata(asset->handle);
    md.loaded = true;
    md.asset = asset;
    asset->SetFlag(AssetFlag::ASSET_LOADED, true);
  }

  void AssetDatabase::UnregisterAsset(const AssetMetadata& metadata) {
    OE_ASSERT(metadata.handle != 0, "Invalid asset metadata");
    UnregisterAsset(metadata.handle);
  }

  void AssetDatabase::UnregisterAsset(const AssetHandle metadata) {
    OE_ASSERT(metadata != 0, "Invalid asset metadata");
    asset_registry.RemoveAsset(metadata);
  }

  AssetHandle AssetDatabase::GetHandle(const AssetKey& key) {
    if (!HasKey(key)) {
      OE_ERROR("Asset key not found : {}", key.file_handle);
      return 0;
    }

    return asset_registry.GetMetadata(key).handle;
  }

  AssetMetadata& AssetDatabase::Get(const AssetKey& key) {
    OE_ASSERT(HasKey(key), "Asset key not found : {}", key.file_handle);
    return asset_registry.GetMetadata(key);
  }

  AssetMetadata& AssetDatabase::Get(AssetHandle handle) {
    OE_ASSERT(Contains(handle), "Asset not found : {}", handle);
    return asset_registry.GetMetadata(handle);
  }

  std::set<AssetHandle> AssetDatabase::GetAllOfType(AssetType type) {
    std::set<AssetHandle> result;
    for (auto& [key, md] : asset_registry.ReadAllAssets()) {
      if (md.type == type) {
        result.insert(md.handle);
      }
    }
    return result;
  }

  // AssetMetadata& AssetDatabase::ProcessAsset(const AssetMetadata& metadata) {
  //   AssetMetadata& md = asset_registry.GetMetadata(metadata.handle);

  //   if (metadata.memory_asset) {
  //     AssetMetadata md = {
  //       .handle = metadata.handle,
  //       .type = metadata.type,
  //       .path = Path(""),
  //       .loaded = true,
  //     };
  //     AddAsset(md);
  //     return asset_registry[md.handle];
  //   }

  //   Path p = metadata.path;
  //   // Ref<FileHandle> fh = Filesystem::GetFile(metadata.path);
  //   // OE_ASSERT(fh != nullptr, "Failed to register file : {}", p.string());
  //   // OE_ASSERT(fh->Exists(), "File does not exist : {}", p.string());

  //   /**
  //    * \note we register two assets for each script, one for the file itself and one for the 'behavior' of the script
  //    *        - for lua these are the same file
  //    *        - for c# the behavior is the compiled assembly (.dll) and the scriptfile asset is the source file (.cs)
  //    **/
  //   if (metadata.type == AssetType::DYNAMIC_LIBRARY && p.extension() == ".cs") {
  //     p.replace_extension(".dll");

  //     AssetMetadata script_md = {
  //       .handle = FNV(p.string()),
  //       .type = AssetType::SCRIPT,
  //       .path = p,
  //       .loaded = false,
  //     };

  //     AssetMetadata file_md = {
  //       .handle = FNV(metadata.path.string()),
  //       .type = AssetType::DYNAMIC_LIBRARY,
  //       .path = metadata.path,
  //       .loaded = false,
  //     };

  //     AddAsset(script_md);
  //     AddAsset(file_md);
  //     return asset_registry[file_md.handle];
  //   }

  //   AssetMetadata md = {
  //     .handle = FNV(p.string()),
  //     .type = metadata.type,
  //     .path = p,
  //     .loaded = false,
  //   };
  //   AddAsset(md);

  //   return asset_registry[md.handle];
  // }

}  // namespace other