/**
 * \file asset/serializers/shader_serializer.hpp
 **/
#ifndef OTHER_ENGINE_SHADER_SERIALIZER_HPP
#define OTHER_ENGINE_SHADER_SERIALIZER_HPP

#include "asset/asset_serializer.hpp"

namespace other {

  class ShaderSerializer : public AssetSerializer {
   public:
    virtual ~ShaderSerializer() override {}

    virtual void Serialize(const AssetMetadata& metadata) override;
    virtual bool Load(AssetMetadata& metadata) override;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_SHADER_SERIALIZER_HPP
