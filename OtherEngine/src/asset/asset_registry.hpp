/**
 * \file asset/asset_registry.hpp
 */
#ifndef OTHER_ENGINE_ASSET_REGISTRY_HPP
#define OTHER_ENGINE_ASSET_REGISTRY_HPP

#include <map>

#include <spdlog/fmt/fmt.h>

#include "asset/asset.hpp"
#include "asset/asset_defines.hpp"

namespace other {

  class FileHandle;

  struct AssetMetadata {
    AssetHandle handle = 0;

    UUID file_handle = 0;
    AssetType type = AssetType::BLANK_ASSET;

    Ref<Asset> asset = nullptr;

    Path path = "";
    bool loaded = false;
    bool memory_asset = false;

    bool IsValid() const;
    bool Loaded() const;
    bool Unloaded() const;
  };

  struct AssetKey {
    UUID file_handle = 0;
    AssetType type = AssetType::BLANK_ASSET;

    constexpr auto operator<=>(const AssetKey&) const = default;
  };

}  // namespace other

template <>
struct fmt::formatter<other::AssetKey> : fmt::formatter<std::string_view> {
  auto format(const other::AssetKey& key, fmt::format_context& ctx) {
    return fmt::formatter<std::string_view>::format(fmtstr("AssetKey({} : {})", key.file_handle, key.type), ctx);
  }
};

template <>
struct fmt::formatter<other::AssetMetadata> : fmt::formatter<std::string_view> {
  auto format(const other::AssetMetadata& meta, fmt::format_context& ctx) {
    // clang-format off
    return fmt::formatter<std::string_view>::format(fmtstr("AssetMetadata({} : {} : {} : [loaded = {} && memory = {}])", 
                                                    meta.file_handle, meta.type, meta.path, meta.loaded, meta.memory_asset), ctx);
    // clang-format on
  }
};

template <>
struct std::hash<other::AssetKey> {
  std::size_t operator()(const other::AssetKey& key) const {
    return std::hash<uint64_t>{}(key.file_handle.Get()) ^
      std::hash<uint32_t>{}(static_cast<uint32_t>(key.type));
  }
};

template <>
struct std::equal_to<other::AssetKey> {
  bool operator()(const other::AssetKey& lhs, const other::AssetKey& rhs) const {
    return lhs.file_handle == rhs.file_handle && lhs.type == rhs.type;
  }
};

namespace other {

  class AssetRegistry {
   public:
    AssetRegistry() {}
    ~AssetRegistry() {}

    void AddAsset(const FileHandle* file);
    void AddMemoryAsset(const AssetMetadata& metadata);

    bool Contains(AssetHandle handle) const;
    bool HasKey(const AssetKey& key) const;

    AssetMetadata& GetMetadata(AssetHandle handle);
    AssetMetadata& GetMetadata(const AssetKey& key);

    void RemoveAsset(AssetHandle handle);
    void RemoveAsset(const AssetKey& key);

    const std::unordered_map<AssetKey, AssetMetadata>& ReadAllAssets() const;

   private:
    std::unordered_map<AssetKey, AssetMetadata> assets;

    friend class AssetDatabase;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_REGISTRY_HPP
