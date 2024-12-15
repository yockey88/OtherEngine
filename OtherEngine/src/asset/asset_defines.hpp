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
      AssetType::GENERIC_FILE;
      AssetType::MEMORY_ONLY;
      AssetType::SCENE;
      //   AssetType::PREFAB;
      AssetType::MODEL_SOURCE;
      AssetType::MODEL;
      AssetType::SHADER;
      //   AssetType::MATERIAL;
      AssetType::TEXTURE;
      //   AssetType::ENVMAP;
      //   AssetType::AUDIO;
      //   AssetType::SOUNDCONFIG;
      //   AssetType::SPATIALIZATIONCONFIG;
      //   AssetType::FONT;
      AssetType::SCRIPT;
      AssetType::DYNAMIC_LIBRARY;
      //   AssetType::MESHCOLLIDER;
      //   AssetType::SOUNDGRAPHSOUND;
      //   AssetType::SKELETON;
      //   AssetType::ANIMATION;
      //   AssetType::ANIMATIONGRAPH;
      AssetType::SOURCEFILE;
      return AssetType::INVALID_ASSET;
    }

  }  // namespace util

  static const std::map<UUID, AssetType> asset_extensions = {
    /// scene extensions
    { FNV(".yscn"), AssetType::SCENE },

    /// prefabs

    /// mesh/animations
    { FNV(".fbx"), AssetType::MODEL_SOURCE },
    // { FNV(".gltf"), AssetType::MODEL },
    // { FNV(".glb"), AssetType::MODEL },
    { FNV(".obj"), AssetType::MODEL_SOURCE },

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
      case AssetType::MODEL_SOURCE:
        return { ".fbx", ".obj" };  // ".gltf", ".glb",
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

  static constexpr std::array kVirtualDrives = {
    "files://",   /// AssetType::GENERIC_FILE;
    "vfiles://",  /// AssetType::MEMORY_ONLY;
    "scenes://",  // AssetType::SCENE;
    // "prefabs://", //   AssetType::PREFAB;
    "model-sources://",  /// AssetType::MODEL_SOURCE;
    "models://",         /// AssetType::MODEL;
    "shaders://",        /// AssetType::SHADER;
    // "materials://", ///   AssetType::MATERIAL;
    "textures://",  /// AssetType::TEXTURE;
    // "",  /// //   AssetType::ENVMAP;
    // "",  /// //   AssetType::AUDIO;
    // "",  /// //   AssetType::SOUNDCONFIG;
    // "",  /// //   AssetType::SPATIALIZATIONCONFIG;
    // "",  /// //   AssetType::FONT;
    "scripts://",   /// AssetType::SCRIPT;
    "dyn-libs://",  /// AssetType::DYNAMIC_LIBRARY;
    // "",  /// //   AssetType::MESHCOLLIDER;
    // "",  /// //   AssetType::SOUNDGRAPHSOUND;
    // "",  /// //   AssetType::SKELETON;
    // "",  /// //   AssetType::ANIMATION;
    // "",  /// //   AssetType::ANIMATIONGRAPH;
    "sourcefiles://",  /// AssetType::SOURCEFILE;
  };

  static std::string_view VirtualDriveFromAssetType(AssetType type) {
    if (type >= AssetType::NUM_ASSET_TYPES) {
      return "";
    }
    return kVirtualDrives[type];
  }

  static std::string_view VirtualDriveFromExtension(std::string_view ext) {
    auto itr = asset_extensions.find(FNV(ext));
    if (itr == asset_extensions.end()) {
      return "";
    }
    return VirtualDriveFromAssetType(itr->second);
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