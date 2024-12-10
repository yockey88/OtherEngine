/**
 * \file asset/asset_defines.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_DEFINES_HPP
#define OTHER_ENGINE_ASSET_DEFINES_HPP

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <string_view>

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/uuid.hpp"

namespace other {

  class Asset;

  struct AssetHandle {
    UUID id;

    uint64_t Get() const {
      return id.Get();
    }

    constexpr AssetHandle()
        : id(0) {}

    constexpr AssetHandle(uint64_t uid) {
      id = uid;
    }

    constexpr AssetHandle(UUID id)
        : id(id) {}
  };

  template <>
  constexpr ValueType GetValueType<AssetHandle>() {
    return ValueType::ASSET;
  }

  inline bool operator==(const AssetHandle& lhs, const AssetHandle& rhs) {
    return lhs.id == rhs.id;
  }

  enum AssetFlag : uint16_t {
    NO_ASSET_FLAGS = 0,
    READ_ONLY = bit(0),
    READ_WRITE = bit(1),
    MISSING = bit(2),
    DIRTY = bit(3),
    ASSET_INVALID = bit(4),
    ASSET_LOADED = bit(5),
  };

  enum AssetType : uint16_t {
    BLANK_ASSET = 0,
    GENERIC_FILE,
    MEMORY_ONLY,
    SCENE,
    // PREFAB ,
    MODEL_SOURCE,
    MODEL,
    SHADER,
    // MATERIAL ,
    TEXTURE,
    // ENVMAP ,
    // AUDIO ,
    // SOUNDCONFIG ,
    // SPATIALIZATIONCONFIG ,
    // FONT ,
    SCRIPT,
    DYNAMIC_LIBRARY,
    // MESHCOLLIDER ,
    // SOUNDGRAPHSOUND ,
    // SKELETON ,
    // ANIMATION ,
    // ANIMATIONGRAPH ,
    SOURCEFILE,

    NUM_ASSET_TYPES,
    INVALID_ASSET = 0xFFFF
  };

  struct Source {
    std::string name;
    std::vector<Path> files;
  };

  struct Resource {
    std::string name;
    AssetType type;
    AssetHandle handle;
  };

  namespace util {

    static AssetType AssetTypeFromString(std::string_view asset_str) {
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
      if (asset_str == "DYNAMIC_LIBRARY") return AssetType::DYNAMIC_LIBRARY;
      // if (asset_str == "MESH-COLLIDER") return AssetType::MESHCOLLIDER;
      // if (asset_str == "SOUND-GRAPH-SOUND") return AssetType::SOUNDGRAPHSOUND;
      // if (asset_str == "SKELETON") return AssetType::SKELETON;
      // if (asset_str == "ANIMATION") return AssetType::ANIMATION;
      // if (asset_str == "ANIMATION-GRAPH") return AssetType::ANIMATIONGRAPH;
      if (asset_str == "SOURCEFILE") return AssetType::SOURCEFILE;
      return AssetType::INVALID_ASSET;
    }

  }  // namespace util

  static const std::map<UUID, AssetType> asset_extensions = {
    /// scene extensions
    { FNV(".yscn"), AssetType::SCENE },

    /// prefabs

    /// mesh/animations
    { FNV(".fbx"), AssetType::MODEL },
    { FNV(".gltf"), AssetType::MODEL },
    { FNV(".glb"), AssetType::MODEL },
    { FNV(".obj"), AssetType::MODEL },

    /// shaders
    { FNV(".glsl"), AssetType::SHADER },
    { FNV(".vert"), AssetType::SHADER },
    { FNV(".frag"), AssetType::SHADER },
    { FNV(".geom"), AssetType::SHADER },
    { FNV(".oshader"), AssetType::SHADER },

    /// materials

    /// textures
    { FNV(".png"), AssetType::TEXTURE },
    { FNV(".jpg"), AssetType::TEXTURE },
    { FNV(".jpeg"), AssetType::TEXTURE },

    /// env maps
    /// audio
    /// fonts
    /// { FNV(".ttf") , AssetType::FONT } ,
    /// { FNV(".ttc") , AssetType::FONT } ,
    /// { FNV(".otf") , AssetType::FONT } ,

    /// scripts
    { FNV(".cs"), AssetType::SCRIPT },
    { FNV(".lua"), AssetType::SCRIPT },
    { FNV(".py"), AssetType::SCRIPT },

    /// loadable libraries
    { FNV(".dll"), AssetType::DYNAMIC_LIBRARY },

    // source
    { FNV(".h"), AssetType::SOURCEFILE },
    { FNV(".hpp"), AssetType::SOURCEFILE },
    { FNV(".cpp"), AssetType::SOURCEFILE },
  };

  static std::vector<std::string> GetAssetTypeExtensions(AssetType type) {
    switch (type) {
      case AssetType::SCENE:
        return { ".yscn" };
      case AssetType::MODEL:
        return { ".fbx", ".gltf", ".glb", ".obj" };
      case AssetType::SHADER:
        return { ".glsl", ".vert", ".frag", ".geom", ".oshader" };
      case AssetType::TEXTURE:
        return { ".png", ".jpg", ".jpeg" };
      case AssetType::SCRIPT:
        return { ".cs", ".lua" };
      case AssetType::DYNAMIC_LIBRARY:
        return { ".dll" };
      case AssetType::SOURCEFILE:
        return { ".h", ".hpp", ".cpp" };
      default:
        return {};
    }
  }

}  // namespace other

template <>
struct fmt::formatter<other::AssetHandle> : public fmt::formatter<std::string_view> {
  auto format(const other::AssetHandle& handle, fmt::format_context& ctx) {
    return fmt::formatter<std::string_view>::format(fmt::format(std::string_view{ "[{}:{:#08x}]" }, handle.Get(), handle.Get()), ctx);
  }
};

namespace std {

  template <>
  struct hash<other::AssetHandle> {
    size_t operator()(const other::AssetHandle& handle) const {
      return hash<other::UUID>{}(handle.id);
    }
  };

  template <>
  struct less<other::AssetHandle> {
    size_t operator()(const other::AssetHandle& lhs, const other::AssetHandle& rhs) const {
      return lhs.Get() < rhs.Get();
    }
  };

}  // namespace std

#endif  // !OTHER_ENGINE_ASSET_DEFINES_HPP