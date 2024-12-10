/**
 * \file asset/asset_serializer.cpp
 **/
#include "asset/asset_serializer.hpp"

#include "application/app_state.hpp"

namespace other {

  void AssetSerializer::Serialize(Ref<Asset>& asset) {
    OE_ASSERT(asset != nullptr, "Attempting to serialize a null asset");
    OE_ASSERT(asset->GetAssetType() == AssetType::SCENE, "Attempting to serialize a non-scene asset as a scene");

    AssetMetadata& metadata = AppState::Assets()->GetMutableMetadata(asset->handle);
    Serialize(metadata);
  }

}  // namespace other
