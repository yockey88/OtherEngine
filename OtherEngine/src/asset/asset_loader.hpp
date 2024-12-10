/**
 * \file asset/asset_loader.hpp
 */
#ifndef OTHER_ENGINE_ASSET_LOADER_HPP
#define OTHER_ENGINE_ASSET_LOADER_HPP

#include "core/ref.hpp"

#include "asset/asset.hpp"
#include "asset/asset_registry.hpp"
#include "asset/asset_serializer.hpp"

namespace other {

  class AssetLoader {
   public:
    static void Serialize(const Ref<Asset>& asset);
    static void Serialize(const AssetMetadata& metadata);
    static Ref<Asset> Load(AssetMetadata& metadata);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_LOADER_HPP
