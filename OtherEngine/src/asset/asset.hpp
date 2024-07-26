/**
 * \file asset/asset.hpp
 **/
#ifndef OTHER_ENGINE_ASSET_HPP
#define OTHER_ENGINE_ASSET_HPP

#include <type_traits>

#include "core/ref_counted.hpp"
#include "asset/asset_types.hpp"

namespace other {

#define OE_ASSET(asset_type) \
  static AssetType GetStaticType() { return AssetType::asset_type; } \
  virtual AssetType GetAssetType() const override { return AssetType::asset_type; } 

  class Asset : public RefCounted {
    public:
      Asset() {}
      virtual ~Asset() {}

      static AssetType GetStaticType();  
      virtual AssetType GetAssetType() const;

      virtual bool operator==(const Asset& other) const;
      virtual bool operator!=(const Asset& other) const;

      AssetHandle handle = 0;
      uint16_t flags = AssetFlag::NO_ASSET_FLAGS;

    private:
      bool IsValid() const;

      bool CheckFlag(AssetFlag flag) const;
      void SetFlag(AssetFlag flag , bool val = true);

      friend class EditorAssetHandler;
      friend class RuntimeAssetHandler;
  };

  template <typename T>
  concept asset_t = std::is_base_of_v<Asset , T>;

} // namespace other

#endif // !OTHER_ENGINE_ASSET_HPP
