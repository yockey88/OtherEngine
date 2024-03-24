/**
 * \file asset/asset.cpp
 */
#include "asset/asset.hpp"

namespace other {

  Asset::~Asset() {
    Decrement();
  }

  Asset::Asset(Asset&& other) {
    asset_handle = other.asset_handle;
    flags = other.flags;
    other.asset_handle = 0;
    other.flags = AssetFlag::NO_ASSET_FLAGS;
  }

  Asset::Asset(const Asset& other) {
    asset_handle = other.asset_handle;
    flags = other.flags;
    Increment();
  }

  Asset& Asset::operator=(Asset&& other) {
    asset_handle = other.asset_handle;
    flags = other.flags;
    other.asset_handle = 0;
    other.flags = AssetFlag::NO_ASSET_FLAGS;
    return *this;
  }

  Asset& Asset::operator=(const Asset& other) {
    asset_handle = other.asset_handle;
    flags = other.flags;
    Increment();
    return *this;
  }

} // namespace other
