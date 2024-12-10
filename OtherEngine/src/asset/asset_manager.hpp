/**
 * \file asset/asset_manager.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_MANAGER_HPP
#define OTHER_ENGINE_ASSET_MANAGER_HPP

#include "application/app_state.hpp"
#include "asset/asset.hpp"

namespace other {

  class AssetManager {
   public:
    template <asset_t A, typename... Args>
    static AssetHandle CreateMemOnly(const std::string_view name, Args&&... args) {
      Ref<Asset> asset = Ref<A>::Create(std::forward<Args>(args)...);
      AppState::Assets()->AddMemOnly(name, asset);
      return asset->handle;
    }

    template <asset_t A>
    static Ref<A> GetAsset(AssetHandle handle) {
      Ref<Asset> asset = AppState::Assets()->GetAsset(handle);
      return Ref<Asset>::Cast<A>(asset);
    }

    template <asset_t A>
    static Ref<A> GetAsset(UUID file_handle, AssetType type) {
      Ref<Asset> asset = AppState::Assets()->GetAsset(file_handle, type);
      return Ref<Asset>::Cast<A>(asset);
    }

    template <asset_t A>
    static Ref<A> GetAsset(const AssetKey& key) {
      Ref<Asset> asset = AppState::Assets()->GetAsset(key);
      return Ref<Asset>::Cast<A>(asset);
    }

    static AssetType AssetTypeFromExtension(const std::string_view extension);
    static bool AssetValid(AssetHandle handle);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_MANAGER_HPP
