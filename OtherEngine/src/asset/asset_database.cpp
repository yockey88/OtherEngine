/**
 * \file asset/asset_database.cpp
 **/
#include "asset/asset_database.hpp"

#include "core/defines.hpp"
#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "asset/asset_metadata.hpp"
#include "asset/asset_registry.hpp"
#include "asset/asset_types.hpp"

namespace other {
  namespace {

    static AssetRegistry asset_registry;

  }  // anonymous namespace

  void AssetDatabase::RegisterAsset(const AssetMetadata& metadata) {
    OE_ASSERT(metadata.handle != 0, "Invalid asset metadata");
    OE_ASSERT(metadata.type != AssetType::BLANK_ASSET, "Invalid asset type {}", metadata.type);

    {
      auto itr = asset_registry.find(metadata.handle);
      if (itr != asset_registry.end()) {
        OE_WARN("overwriting asset metadata : {}", metadata.handle);
        /// ask if we want to overwrite??
      }
    }

    auto& md = ProcessAsset(metadata);
    OE_DEBUG("Registered asset [{}] : {} ({})", md.type, md.handle, md.path.string() == "" ? "memory" : md.path.string());
  }

  void AssetDatabase::UnregisterAsset(const AssetMetadata& metadata) {
    OE_ASSERT(metadata.handle != 0, "Invalid asset metadata");
    UnregisterAsset(metadata.handle);
  }

  void AssetDatabase::UnregisterAsset(const AssetHandle metadata) {
    OE_ASSERT(metadata != 0, "Invalid asset metadata");

    auto itr = asset_registry.find(metadata);
    if (itr == asset_registry.end()) {
      OE_WARN("Asset not registered : {}", metadata);
      return;
    }

    OE_DEBUG("Unregistered asset : {}", metadata);
    asset_registry.assets.erase(itr);
  }

  bool AssetDatabase::Contains(AssetHandle handle) {
    return asset_registry.Contains(handle);
  }

  AssetMetadata& AssetDatabase::Get(AssetHandle handle) {
    OE_ASSERT(Contains(handle), "Asset not found : {}", handle);
    return asset_registry[handle];
  }

  AssetMetadata& AssetDatabase::ProcessAsset(const AssetMetadata& metadata) {
    {
      auto itr = asset_registry.find(metadata.handle);
      OE_ASSERT(itr == asset_registry.end(), "Asset extension not found : {}", metadata.handle);
    }

    if (metadata.memory_asset) {
      AssetMetadata md = {
        .handle = metadata.handle,
        .type = metadata.type,
        .path = Path(""),
        .loaded = true,
      };
      AddAsset(md);
      return asset_registry[md.handle];
    }

    Path p = metadata.path;
    // Ref<FileHandle> fh = Filesystem::GetFile(metadata.path);
    // OE_ASSERT(fh != nullptr, "Failed to register file : {}", p.string());
    // OE_ASSERT(fh->Exists(), "File does not exist : {}", p.string());

    /**
     * \note we register two assets for each script, one for the file itself and one for the 'behavior' of the script
     *        - for lua these are the same file
     *        - for c# the behavior is the compiled assembly (.dll) and the scriptfile asset is the source file (.cs)
     **/

    /// if it is a a C# script file we want to point it to path-no-ext + .dll so the asset points
    ///   to the compiled assembly
    /// for a script we return the script file asset
    if (metadata.type == AssetType::SCRIPTFILE && p.extension() == ".cs") {
      p.replace_extension(".dll");

      AssetMetadata script_md = {
        .handle = FNV(p.string()),
        .type = AssetType::SCRIPT,
        .path = p,
        .loaded = false,
      };

      AssetMetadata file_md = {
        .handle = FNV(metadata.path.string()),
        .type = AssetType::SCRIPTFILE,
        .path = metadata.path,
        .loaded = false,
      };

      AddAsset(script_md);
      AddAsset(file_md);
      return asset_registry[file_md.handle];
    }

    AssetMetadata md = {
      .handle = FNV(p.string()),
      .type = metadata.type,
      .path = p,
      .loaded = false,
    };
    AddAsset(md);

    return asset_registry[md.handle];
  }

  void AssetDatabase::AddAsset(const AssetMetadata& metadata) {
    OE_ASSERT(metadata.handle != 0, "Invalid asset metadata");
    OE_ASSERT(metadata.type != AssetType::BLANK_ASSET, "Invalid asset metadata");
    OE_ASSERT(asset_registry.find(metadata.handle) == asset_registry.end(), "Asset already registered");
    asset_registry[metadata.handle] = metadata;
  }

}  // namespace other