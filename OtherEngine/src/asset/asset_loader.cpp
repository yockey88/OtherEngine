/**
 * \file asset/asset_loader.cpp
 */
#include "asset/asset_loader.hpp"

#include "core/logger.hpp"

#include "scene/scene_serializer.hpp"

namespace other {
  namespace {

    constexpr static std::array<Scope<AssetSerializer> (*)(), NUM_ASSET_TYPES> asset_loaders = {
      // AssetType::BLANK_ASSET
      []() -> Scope<AssetSerializer> { OE_ASSERT(false, "Cannot load a blank asset"); return nullptr; },
      // AssetType::GENERIC_FILE
      []() -> Scope<AssetSerializer> { OE_ASSERT(false, "unimplemented (load a FileHandle and mount in filesystem)");return nullptr; },
      // AssetType::MEMORY_ONLY
      []() -> Scope<AssetSerializer> { OE_ASSERT(false, "Asset is loaded in memory!"); return nullptr; },
      []() -> Scope<AssetSerializer> { return NewScope<SceneSerializer>(); },  // AssetType::SCENE
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::PREFAB
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::MODEL_SOURCE
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::MODEL
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SHADER
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::MATERIAL
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::TEXTURE
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::ENVMAP
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::AUDIO
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SOUNDCONFIG
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SPATIALIZATIONCONFIG
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::FONT
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SCRIPT
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::DYNAMIC_LIBRARY
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::MESHCOLLIDER
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SOUNDGRAPHSOUND
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SKELETON
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::ANIMATION
      // []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::ANIMATIONGRAPH
      []() -> Scope<AssetSerializer> { return nullptr; },  // AssetType::SOURCEFILE
    };

  }  // namespace

  void AssetLoader::Serialize(const Ref<Asset>& asset) {
    OE_ASSERT(false, "AssetLoader::Serialize unimplemented");
  }

  void AssetLoader::Serialize(const AssetMetadata& metadata) {
    OE_ASSERT(false, "AssetLoader::Serialize unimplemented");
  }

  Ref<Asset> AssetLoader::Load(AssetMetadata& metadata) {
    OE_ASSERT(metadata.type != AssetType::BLANK_ASSET, "Cannot load a blank asset");
    OE_ASSERT(metadata.type < NUM_ASSET_TYPES, "Invalid asset type : {}", metadata.type);
    OE_ASSERT(asset_loaders[metadata.type] != nullptr, "Asset loader not implemented for type : {}", metadata.type);
    OE_ASSERT(!metadata.loaded && !metadata.memory_asset, "Asset already loaded : {}", metadata.handle);

    Scope<AssetSerializer> loader = asset_loaders[metadata.type]();
    OE_ASSERT(loader != nullptr, "Failed to create asset loader for type : {}", metadata.type);

    if (loader->Load(metadata)) {
      return metadata.asset;
    }

    OE_ERROR("Failed to load asset : {}", metadata.handle);
    return nullptr;
  }

}  // namespace other
