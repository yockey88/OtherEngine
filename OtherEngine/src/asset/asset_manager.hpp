/**
 * \file asset/asset_manager.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_MANAGER_HPP
#define OTHER_ENGINE_ASSET_MANAGER_HPP

#include "application/app_state.hpp"

namespace other {

  class AssetManager {
    public:
      template <asset_t A , typename... Args>
      static AssetHandle CreateMemOnly(Args&&... args) {
        Ref<Asset> asset = Ref<A>::Create(std::forward<Args>(args)...);
        AppState::Assets()->AddMemOnly(asset);
        return asset->handle;
      }
    
      template <asset_t A>
      static Ref<A> GetAsset(AssetHandle handle) {
        Ref<Asset> asset = AppState::Assets()->GetAsset(handle);
        return Ref<Asset>::Cast<A>(asset);
      }
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_MANAGER_HPP
