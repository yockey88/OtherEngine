/**
 * \file asset/serializers/model_serializer.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_SERIALIZER_HPP
#define OTHER_ENGINE_MODEL_SERIALIZER_HPP

#include "asset/asset_serializer.hpp"

#include "rendering/vertex.hpp"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace other {

  class ModelSerializer : public AssetSerializer {
   public:
    virtual ~ModelSerializer() override {}

    virtual void Serialize(const AssetMetadata& metadata) override;
    virtual bool Load(AssetMetadata& metadata) override;

   protected:
    struct ModelData {
      std::string name;
      std::vector<Vertex> vertices;
      std::vector<Index> triangles;

      uint32_t start_vertex = 0;
      uint32_t start_index = 0;
      uint32_t idx_cnt = 0;
      uint32_t vert_cnt = 0;

      /// bone data
      /// submesh data
    };
    std::vector<ModelData> models;

    std::stack<uint32_t> index_stack;

    // std::vector<Vertex> vertices;
    // std::vector<Index> triangles;

    virtual bool ProcessScene(const aiScene* scene, const std::string& path);
    virtual bool ProcessNode(const aiNode* node, const aiScene* scene);
    virtual bool ProcessMesh(const aiMesh* mesh, const aiScene* scene);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MODEL_SERIALIZER_HPP
