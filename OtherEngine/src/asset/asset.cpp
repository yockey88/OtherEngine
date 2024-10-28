/**
 * \file asset/asset.cpp
 */
#include "asset/asset.hpp"

#include "core/rand.hpp"

namespace other {

  Asset::Asset() {
    handle = Random::GenerateUUID();
  }

  AssetType Asset::GetStaticType() {
    OE_ASSERT(false, "Asset::GetStaticType() called on base class");
  }

  bool Asset::operator==(const Asset& other) const {
    return handle == other.handle;
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

  void Asset::SetFlag(AssetFlag flag, bool val) {
    if (val) {
      flags |= flag;
    } else {
      flags &= ~flag;
    }
  }

}  // namespace other
