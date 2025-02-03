/**
 * \file asset/asset.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_HPP
#define OTHER_ENGINE_ASSET_HPP

#include <type_traits>

#include "core/file_handle.hpp"
#include "core/ref_counted.hpp"

#include "asset/asset_defines.hpp"

namespace other {

#define OE_ASSET(asset_type)                                         \
  static AssetType GetStaticType() { return AssetType::asset_type; } \
  virtual AssetType GetAssetType() const override { return AssetType::asset_type; }

  class Asset : public RefCounted {
   public:
    Asset();
    virtual ~Asset() {}

    static AssetType GetStaticType();
    virtual AssetType GetAssetType() const = 0;

    const Ref<FileHandle> GetFileHandle() const;

    virtual bool operator==(const Asset& other) const;
    virtual bool operator!=(const Asset& other) const;

    AssetHandle handle = 0;
    uint16_t flags = AssetFlag::NO_ASSET_FLAGS;

   private:
    friend class AssetLoader;

    bool IsValid() const;

    bool CheckFlag(AssetFlag flag) const;
    void SetFlag(AssetFlag flag, bool val = true);

    Ref<FileHandle> file_handle = nullptr;

    friend struct AssetMetadata;
    friend class AssetRegistry;
    friend class AssetDatabase;

    friend class AssetHandler;
    friend class EditorAssetHandler;
    friend class RuntimeAssetHandler;
  };

  template <typename T>
  concept asset_t = std::is_base_of_v<Asset, T>;

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_HPP
