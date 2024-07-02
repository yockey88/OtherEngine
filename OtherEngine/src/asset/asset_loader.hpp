/**
 * \file asset/asset_loader.hpp
*/
#ifndef OTHER_ENGINE_ASSET_LOADER_HPP
#define OTHER_ENGINE_ASSET_LOADER_HPP

#include "core/ref.hpp"
#include "asset/asset.hpp"
#include "asset/asset_metadata.hpp"
#include "asset/asset_serializer.hpp"

namespace other {

  class AssetLoader {
    public:
      static void Serialize(const Ref<Asset>& asset);
      static void Serialize(const AssetMetadata& metadata , const Ref<Asset>& asset);
      static bool Load(const AssetMetadata& metadata , Ref<Asset>& asset);

      static bool TryLoad(const AssetMetadata& metadata , Ref<Asset>& asset);

    private:
      static std::unordered_map<AssetType , Scope<AssetSerializer>> asset_loaders;
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_LOADER_HPP
