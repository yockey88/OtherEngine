/**
 * \file asset/asset_types.hpp
 */
#ifndef OTHER_ENGINE_ASSET_TYPES_HPP
#define OTHER_ENGINE_ASSET_TYPES_HPP

#include <map>
#include <set>
#include <string_view>

#include "core/ref.hpp"
#include "core/uuid.hpp"

#include "asset/asset_defines.hpp"

namespace other {

  using AssetSet = std::set<AssetHandle>;
  using AssetMap = std::map<AssetHandle, Ref<Asset>>;

  namespace util {

    inline AssetType AssetTypeFromString(std::string_view asset_str) {
      if (asset_str == "GENERIC-FILE") return AssetType::GENERIC_FILE;
      if (asset_str == "MEMORY-ONLY") return AssetType::MEMORY_ONLY;
      if (asset_str == "SCENE") return AssetType::SCENE;
      // if (asset_str == "PREFAB") return AssetType::PREFAB;
      if (asset_str == "MODEL-SOURCE") return AssetType::MODEL_SOURCE;
      if (asset_str == "MODEL") return AssetType::MODEL;
      if (asset_str == "SHADER") return AssetType::SHADER;
      // if (asset_str == "MATERIAL") return AssetType::MATERIAL;
      if (asset_str == "TEXTURE") return AssetType::TEXTURE;
      // if (asset_str == "ENV-MAP") return AssetType::ENVMAP;
      // if (asset_str == "AUDIO") return AssetType::AUDIO;
      // if (asset_str == "SOUND-CONFIG") return AssetType::SOUNDCONFIG;
      // if (asset_str == "SPATIALIZATION-CONFIG") return AssetType::SPATIALIZATIONCONFIG;
      // if (asset_str == "FONT") return AssetType::FONT;
      if (asset_str == "SCRIPT") return AssetType::SCRIPT;
      if (asset_str == "SCRIPTFILE") return AssetType::SCRIPTFILE;
      // if (asset_str == "MESH-COLLIDER") return AssetType::MESHCOLLIDER;
      // if (asset_str == "SOUND-GRAPH-SOUND") return AssetType::SOUNDGRAPHSOUND;
      // if (asset_str == "SKELETON") return AssetType::SKELETON;
      // if (asset_str == "ANIMATION") return AssetType::ANIMATION;
      // if (asset_str == "ANIMATION-GRAPH") return AssetType::ANIMATIONGRAPH;
      if (asset_str == "SOURCEFILE") return AssetType::SOURCEFILE;
      return AssetType::INVALID_ASSET;
    }

  }  // namespace util
}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_TYPES_HPP
