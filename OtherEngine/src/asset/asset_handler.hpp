/**
 * \file asset/asset_handler.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_HANDLER_HPP
#define OTHER_ENGINE_ASSET_HANDLER_HPP

#include "core/ref_counted.hpp"
#include "core/ref.hpp"

#include "asset/asset.hpp"
#include "asset/asset_types.hpp"

namespace other {

  class AssetHandler : public RefCounted {
    public:
      AssetHandler() {}
      virtual ~AssetHandler() {}

      virtual AssetType GetAssetType(AssetHandle assetHandle) = 0;
      virtual Ref<Asset> GetAsset(AssetHandle assetHandle) = 0;
      virtual void AddMemoryOnlyAsset(Ref<Asset> asset) = 0;
      virtual bool ReloadData(AssetHandle assetHandle) = 0;

      // the asset handle is valid (this says nothing about the asset itself)
      virtual bool IsAssetHandleValid(AssetHandle assetHandle) = 0; 
      // asset exists in memory only, there is no backing file
      virtual bool IsMemoryAsset(AssetHandle handle) = 0;           
      // asset has been loaded from file (it could still be invalid)
      virtual bool IsAssetLoaded(AssetHandle handle) = 0;           
      // asset file was loaded, but is invalid for some reason (e.g. corrupt file)
      virtual bool IsAssetValid(AssetHandle handle) = 0;            
      // asset file is missing
      virtual bool IsAssetMissing(AssetHandle handle) = 0;          

      virtual void RemoveAsset(AssetHandle handle) = 0;
      
      virtual AssetSet GetAllAssetsOfType(AssetType type) = 0;
      virtual const AssetMap& GetAllAssets() = 0;
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_HANDLER_HPP
