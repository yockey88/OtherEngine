/**
 * \file asset/asset.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_HPP
#define OTHER_ENGINE_ASSET_HPP

#include "core/ref_counted.hpp"
#include "asset/asset_types.hpp"

namespace other {

  class Asset : public RefCounted {
    public:
      Asset() {}
      virtual ~Asset();

      Asset(Asset&& other);
      Asset(const Asset& other);
      Asset& operator=(Asset&& other);
      Asset& operator=(const Asset& other);

      static AssetType GetStaticType() { return AssetType::BLANK_ASSET; }  
      virtual AssetType GetAssetType() const { return AssetType::BLANK_ASSET; }

      virtual bool operator==(const Asset& other) const { return asset_handle == other.asset_handle; }
      virtual bool operator!=(const Asset& other) const { return !(*this == other); }

      AssetHandle asset_handle = 0;
      uint16_t flags = AssetFlag::NO_ASSET_FLAGS;

    private:
      bool IsValid() const { return (!CheckFlag(AssetFlag::ASSET_INVALID) && !CheckFlag(AssetFlag::MISSING)); }

      bool CheckFlag(AssetFlag flag) const { return (flags & flag); }
      void SetFlag(AssetFlag flag , bool val = true) {
        if (val) {
          flags |= flag;
        } else {
          flags &= ~flag;
        }
      }

      // friend class EditorAssetManager;
      friend class MeshLoader;
      friend class TextureSerializer;
      friend class AssetHandler;
  };

} // namespace other

#endif // !OTHER_ENGINE_ASSET_HPP
