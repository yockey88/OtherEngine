/**
 * \file asset/asset_serializer.hpp
 */
#ifndef OTHER_ENGINE_ASSET_SERIALIZER_HPP
#define OTHER_ENGINE_ASSET_SERIALIZER_HPP

#include "core/ref.hpp"

#include "asset/asset.hpp"
#include "asset/asset_registry.hpp"

#include "serialization/serializer.hpp"

namespace other {

  class AssetSerializer : public Serializer {
   public:
    virtual ~AssetSerializer() {}

    void Serialize(Ref<Asset>& asset);
    virtual void Serialize(const AssetMetadata& metadata) = 0;
    virtual bool Load(AssetMetadata& metadata) = 0;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_ASSET_SERIALIZER_HPP
