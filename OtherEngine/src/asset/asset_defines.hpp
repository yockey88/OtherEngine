/**
 * \file asset/asset_defines.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_DEFINES_HPP
#define OTHER_ENGINE_ASSET_DEFINES_HPP

#include <cstdint>
#include <string>

#include "core/defines.hpp"
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
    SCRIPTFILE,
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