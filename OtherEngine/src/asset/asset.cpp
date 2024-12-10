/**
 * \file asset/asset.cpp
 */
#include "asset/asset.hpp"

#include "core/logger.hpp"
#include "core/rand.hpp"

#include "asset/asset_database.hpp"

namespace other {

  Asset::Asset() {
    handle = Random::GenerateUUID();
  }

  AssetType Asset::GetStaticType() {
    OE_ASSERT(false, "Asset::GetStaticType() called on base class");
    return AssetType::BLANK_ASSET;
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
