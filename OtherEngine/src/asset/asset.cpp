/**
 * \file asset/asset.cpp
 */
#include "asset/asset.hpp"

namespace other {

  AssetType Asset::GetStaticType() { 
    return AssetType::BLANK_ASSET; 
  }  

  AssetType Asset::GetAssetType() const { 
    return AssetType::BLANK_ASSET; 
  }

  bool Asset::operator==(const Asset& other) const { 
    return asset_handle == other.asset_handle; 
  }

  bool Asset::operator!=(const Asset& other) const { 
    return !(*this == other); 
  }

  bool Asset::IsValid() const { 
    return (!CheckFlag(AssetFlag::ASSET_INVALID) && !CheckFlag(AssetFlag::MISSING)); 
  }

  bool Asset::CheckFlag(AssetFlag flag) const { 
    return (flags & flag); 
  }

  void Asset::SetFlag(AssetFlag flag , bool val) {
    if (val) {
      flags |= flag;
    } else {
      flags &= ~flag;
    }
  }

} // namespace other
