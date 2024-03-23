/**
 * \file asset/asset_types.hpp
*/
#ifndef OTHER_ENGINE_ASSET_TYPES_HPP
#define OTHER_ENGINE_ASSET_TYPES_HPP

#include <array>
#include <string_view>

#include "core/defines.hpp"
#include "core/uuid.hpp"

namespace other {

  using AssetHandle = UUID;

  enum AssetFlag : uint16_t {
    NO_ASSET_FLAGS = 0 , 
    READ_ONLY = bit(0) ,
    READ_WRITE = bit(1) ,
    MISSING = bit(2) ,
    DIRTY = bit(3) ,
    ASSET_INVALID = bit(4) ,
    ASSET_LOADED = bit(5) ,
  };

  enum AssetType : uint16_t {
    BLANK_ASSET = 0 ,
    SCENE ,
    PREFAB ,
    MESH ,
    SHADER ,
    MATERIAL ,
    TEXTURE ,
    ENVMAP ,
    AUDIO ,
    SOUNDCONFIG ,
    SPATIALIZATIONCONFIG ,
    FONT ,
    SCRIPT ,
    SCRIPTFILE ,
    MESHCOLLIDER , 
    SOUNDGRAPHSOUND ,
    SKELETON ,
    ANIMATION , 
    ANIMATIONGRAPH ,

    NUM_ASSET_TYPES ,
    INVALID_ASSET = 0xFFFF
  };

namespace util {

  inline AssetType AssetTypeFromString(std::string_view asset_str) {
    if (asset_str == "SCENE") return AssetType::SCENE;
    if (asset_str == "PREFAB") return AssetType::PREFAB;
    if (asset_str == "MESH") return AssetType::MESH;
    if (asset_str == "SHADER") return AssetType::SHADER;
    if (asset_str == "MATERIAL") return AssetType::MATERIAL;  
    if (asset_str == "TEXTURE") return AssetType::TEXTURE;
    if (asset_str == "ENV-MAP") return AssetType::ENVMAP;
    if (asset_str == "AUDIO") return AssetType::AUDIO;
    if (asset_str == "SOUND-CONFIG") return AssetType::SOUNDCONFIG;
    if (asset_str == "SPATIALIZATION-CONFIG") return AssetType::SPATIALIZATIONCONFIG;
    if (asset_str == "FONT") return AssetType::FONT;
    if (asset_str == "SCRIPT") return AssetType::SCRIPT;
    if (asset_str == "SCRIPT-FILE") return AssetType::SCRIPTFILE;
    if (asset_str == "MESH-COLLIDER") return AssetType::MESHCOLLIDER;
    if (asset_str == "SOUND-GRAPH-SOUND") return AssetType::SOUNDGRAPHSOUND;
    if (asset_str == "SKELETON") return AssetType::SKELETON;
    if (asset_str == "ANIMATION") return AssetType::ANIMATION;
    if (asset_str == "ANIMATION-GRAPH") return AssetType::ANIMATIONGRAPH;
    return AssetType::INVALID_ASSET;
  }

  constexpr static uint16_t kNumAssetTypes = AssetType::NUM_ASSET_TYPES; 
  constexpr static std::array<std::string_view , kNumAssetTypes> kAssetTypeStrings = {
    "SCENE" , 
    "PREFAB" , 
    "MESH" , 
    "SHADER" , 
    "MATERIAL" , 
    "TEXTURE" , 
    "ENV-MAP" , 
    "AUDIO" , 
    "SOUND-CONFIG" , 
    "SPATIALIZATION-CONFIG" , 
    "FONT" , 
    "SCRIPT" , 
    "SCRIPT-FILE" , 
    "MESH-COLLIDER" , 
    "SOUND-GRAPH-SOUND" , 
    "SKELETON" , 
    "ANIMATION" , 
    "ANIMATION-GRAPH" ,

    "INVALID"
  };

} // namespace util
} // namespace other

#endif // !OTHER_ENGINE_ASSET_TYPES_HPP
