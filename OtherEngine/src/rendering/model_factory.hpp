/**
 * \file rendering/model_factory.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_FACTORY_HPP
#define OTHER_ENGINE_MODEL_FACTORY_HPP

#include "asset/asset_types.hpp"

namespace other {

  class ModelFactory {
    public:
      static AssetHandle CreateBox(const glm::vec3& size);
      static AssetHandle CreateSphere(float radius);
      static AssetHandle CreateCapsule(float radius , float height);

  }; 

} // namespace other

#endif // !OTHER_ENGINE_MODEL_FACTORY_HPP
