/**
 * \file rendering/model_factory.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_FACTORY_HPP
#define OTHER_ENGINE_MODEL_FACTORY_HPP

#include "asset/asset_defines.hpp"

#include "rendering/vertex.hpp"

namespace other {

  class ModelFactory {
   public:
    static AssetHandle CreateFramebufferMesh();

    static AssetHandle CreateLine(const glm::vec3& start, const glm::vec3& end);

    static AssetHandle CreateTriangle();
    static AssetHandle CreateRect();

    static AssetHandle CreateBox();
    static AssetHandle CreateBoxWireframe();

    static AssetHandle CreateSphere(float radius = 1.f);
    static AssetHandle CreateCapsule(float radius = 0.25f, float height = 1.f);

   private:
    static Opt<AssetHandle> framebuffer_mesh_handle;

    static Opt<AssetHandle> line_handle;

    static Opt<AssetHandle> triangle_handle;
    static Opt<AssetHandle> rect_handle;

    static Opt<AssetHandle> box_handle;
    static Opt<AssetHandle> box_wireframe_handle;

    static Opt<AssetHandle> sphere_handle;
    static Opt<AssetHandle> capsule_handle;

    static Opt<std::vector<Vertex>> box_vertices;

    static std::vector<Vertex> GetBoxVertices();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MODEL_FACTORY_HPP
