/**
 * \file rendering/model_factory.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_FACTORY_HPP
#define OTHER_ENGINE_MODEL_FACTORY_HPP

#include "asset/asset_types.hpp"

#include "rendering/vertex.hpp"

namespace other {

  class ModelFactory {
    public:
      static AssetHandle CreateTriangle(const glm::vec2& half_extents = { 1.f , 1.f });
      static AssetHandle CreateRect(const glm::vec2& half_extents = { 1.f , 1.f });

      static AssetHandle CreateBox();
      static AssetHandle CreateBoxWireframe();

      static AssetHandle CreateSphere(float radius = 1.f);
      static AssetHandle CreateCapsule(float radius = 0.25f , float height = 1.f);

    private:
      static Opt<AssetHandle> triangle_handle;
      static Opt<AssetHandle> rect_handle;

      static Opt<AssetHandle> box_handle;
      static Opt<AssetHandle> box_wireframe_handle;

      static Opt<AssetHandle> sphere_handle;
      static Opt<AssetHandle> capsule_handle;

      static Opt<std::vector<Vertex>> box_vertices;

      static std::vector<Vertex> GetBoxVertices();
  }; 

} // namespace other

#endif // !OTHER_ENGINE_MODEL_FACTORY_HPP
