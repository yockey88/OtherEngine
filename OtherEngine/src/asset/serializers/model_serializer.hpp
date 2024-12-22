/**
 * \file asset/serializers/model_serializer.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_SERIALIZER_HPP
#define OTHER_ENGINE_MODEL_SERIALIZER_HPP

#include <assimp/material.h>

#include "asset/asset_serializer.hpp"

#include "rendering/material.hpp"
#include "rendering/material_table.hpp"
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
    BBox mesh_bounds = BBox::empty;

    uint32_t vertex_offset = 0;
    uint32_t index_offset = 0;
    std::stack<uint32_t> index_stack;

    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<SubMesh> submeshes;
    std::vector<MeshNode> nodes;

    std::vector<uint64_t> loaded_texture_hashes;

    std::vector<UUID> material_ids;

    virtual void ProcessMaterials(const aiScene* scene);
    virtual bool ProcessNode(const aiNode* node, const aiScene* scene, const uint32_t parent = 0xFFFFFFFF, const glm::mat4& parent_transform = glm::mat4(1.f), uint32_t level = 0);
    virtual bool ProcessMesh(const aiMesh* mesh, const aiScene* scene);

    virtual UUID GetMaterial(const uint32_t idx, const aiScene* scene);
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MODEL_SERIALIZER_HPP
