/**
 * \file asset/asset_metadata.hpp
*/
#ifndef OTHER_ENGINE_ASSET_METADATA_HPP
#define OTHER_ENGINE_ASSET_METADATA_HPP

#include <filesystem>

#include "asset/asset.hpp"

namespace other {

  struct AssetMetadata {
    AssetHandle handle = 0;
		AssetType type = AssetType::BLANK_ASSET;

		Path path = "";
		bool loaded = false;
		bool memory_asset = false;

		bool IsValid() const { return handle != 0 && !memory_asset; }
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_METADATA_HPP