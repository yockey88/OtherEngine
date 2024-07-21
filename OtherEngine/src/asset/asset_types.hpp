/**
 * \file asset/asset_types.hpp
*/
#ifndef OTHER_ENGINE_ASSET_TYPES_HPP
#define OTHER_ENGINE_ASSET_TYPES_HPP

#include <array>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

#include "core/defines.hpp"
#include "core/uuid.hpp"
#include "core/ref.hpp"

namespace other {

  class Asset;

  struct AssetHandle {
    UUID id;

    uint64_t Get() const {
      return id.Get();
    }

    AssetHandle()
      : id(0) {}

    AssetHandle(uint64_t uid) {
      id = uid;
    }

    AssetHandle(UUID id) 
      : id(id) {}
  };

  template <>
  constexpr ValueType GetValueType<AssetHandle>() {
    return ValueType::ASSET;
  }

  inline bool operator==(const AssetHandle& lhs , const AssetHandle& rhs) {
    return lhs.id == rhs.id;
  }

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
    // PREFAB ,
    MODEL_SOURCE , 
    MODEL ,
    MESH ,
    SHADER ,
    // MATERIAL ,
    TEXTURE ,
    // ENVMAP ,
    // AUDIO ,
    // SOUNDCONFIG ,
    // SPATIALIZATIONCONFIG ,
    // FONT ,
    SCRIPTFILE ,
    // MESHCOLLIDER , 
    // SOUNDGRAPHSOUND ,
    // SKELETON ,
    // ANIMATION , 
    // ANIMATIONGRAPH ,

    NUM_ASSET_TYPES ,
    INVALID_ASSET = 0xFFFF
  };
  
  using AssetSet = std::unordered_set<AssetHandle>;
  using AssetMap = std::unordered_map<AssetHandle, Ref<Asset>>;

namespace util {
  

  inline AssetType AssetTypeFromString(std::string_view asset_str) {
    if (asset_str == "SCENE") return AssetType::SCENE;
    // if (asset_str == "PREFAB") return AssetType::PREFAB;
    if (asset_str == "MODEL-SOURCE") return AssetType::MODEL_SOURCE;
    if (asset_str == "MODEL") return AssetType::MODEL;
    if (asset_str == "MESH") return AssetType::MESH;
    if (asset_str == "SHADER") return AssetType::SHADER;
    // if (asset_str == "MATERIAL") return AssetType::MATERIAL;  
    if (asset_str == "TEXTURE") return AssetType::TEXTURE;
    // if (asset_str == "ENV-MAP") return AssetType::ENVMAP;
    // if (asset_str == "AUDIO") return AssetType::AUDIO;
    // if (asset_str == "SOUND-CONFIG") return AssetType::SOUNDCONFIG;
    // if (asset_str == "SPATIALIZATION-CONFIG") return AssetType::SPATIALIZATIONCONFIG;
    // if (asset_str == "FONT") return AssetType::FONT;
    if (asset_str == "SCRIPT-FILE") return AssetType::SCRIPTFILE;
    // if (asset_str == "MESH-COLLIDER") return AssetType::MESHCOLLIDER;
    // if (asset_str == "SOUND-GRAPH-SOUND") return AssetType::SOUNDGRAPHSOUND;
    // if (asset_str == "SKELETON") return AssetType::SKELETON;
    // if (asset_str == "ANIMATION") return AssetType::ANIMATION;
    // if (asset_str == "ANIMATION-GRAPH") return AssetType::ANIMATIONGRAPH;
    return AssetType::INVALID_ASSET;
  }

  constexpr static uint16_t kNumAssetTypes = AssetType::NUM_ASSET_TYPES; 
  constexpr static std::array<std::string_view , kNumAssetTypes> kAssetTypeStrings = {
    "SCENE" , 
    // "PREFAB" , 
    "MODEL-SOURCE" ,
    "MODEL" ,
    "MESH" , 
    "SHADER" , 
    //"MATERIAL" , 
    "TEXTURE" , 
    // "ENV-MAP" , 
    // "AUDIO" , 
    // "SOUND-CONFIG" , 
    // "SPATIALIZATION-CONFIG" , 
    // "FONT" , 
    "SCRIPT-FILE" , 
    // "MESH-COLLIDER" , 
    // "SOUND-GRAPH-SOUND" , 
    // "SKELETON" , 
    // "ANIMATION" , 
    // "ANIMATION-GRAPH" ,

    // "INVALID"
  };

} // namespace util
} // namespace other

template <>
struct fmt::formatter<other::AssetHandle> : public fmt::formatter<std::string_view> {
  auto format(const other::AssetHandle& handle, fmt::format_context& ctx) {
    return fmt::formatter<std::string_view>::format(fmt::format(std::string_view{ "[{}:{:#08x}]" } , handle.Get() , handle.Get()) , ctx);
  }
};

namespace std {

  template<>
  struct hash<other::AssetHandle> {
    size_t operator()(const other::AssetHandle& handle) const {
      return hash<other::UUID>{}(handle.id);
    }
  };
  
  template<>
  struct less<other::AssetHandle> {
    size_t operator()(const other::AssetHandle& lhs , const other::AssetHandle& rhs) const {
      return lhs.Get() < rhs.Get();
    }
  };

} // namespace std

#endif // !OTHER_ENGINE_ASSET_TYPES_HPP
