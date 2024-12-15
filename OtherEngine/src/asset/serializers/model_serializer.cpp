/**
 * \file asset/serializers/model_serializer.cpp
 **/
#include "asset/serializers/model_serializer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "core/logger.hpp"

#include "rendering/model.hpp"

namespace other {

  void ModelSerializer::Serialize(const AssetMetadata& metadata) {}

  bool ModelSerializer::Load(AssetMetadata& metadata) {
    OE_DEBUG("Attempting to load model : {}", metadata.path);
    Assimp::Importer importer;

    uint32_t flags =
      aiProcess_GenSmoothNormals |
      aiProcess_FlipUVs |
      aiProcess_CalcTangentSpace |
      aiProcess_JoinIdenticalVertices |
      aiProcess_SortByPType;
    const aiScene* scene = importer.ReadFile(metadata.path.string(), flags);
    if (scene == nullptr) {
      OE_ERROR("Failed to load model : {}", metadata.path);
      return false;
    }

    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
      OE_ERROR("Failed to load model : {}\n[ASSIMP ERROR : {}]", metadata.path, importer.GetErrorString());
      return false;
    }

    if (!ProcessScene(scene, metadata.path.string())) {
      OE_ERROR("Failed to process model scene : {}", metadata.path);
      return false;
    }

    std::vector<SubMesh> submeshes;
    std::vector<Vertex> source_vertices;
    std::vector<Index> source_indices;

    uint32_t idx_offset = 0;
    uint32_t vert_offset = 0;
    for (ModelData& model : models) {
      OE_DEBUG(" > loaded model : {}", model.name);
      SubMesh& submesh = submeshes.emplace_back();

      submesh.base_vertex = vert_offset;
      submesh.base_idx = idx_offset;
      submesh.model_name = model.name;

      for (Vertex& v : model.vertices) {
        source_vertices.push_back(v);
      }

      for (Index& idx : model.triangles) {
        source_indices.push_back(idx);
      }
      submesh.idx_cnt = source_indices.size();
    }

    metadata.asset = NewRef<ModelSource>(source_vertices, source_indices, submeshes);
    return true;
  }

  bool ModelSerializer::ProcessScene(const aiScene* scene, const std::string& path) {
    OE_ASSERT(scene != nullptr, "Attempting to process a null scene");
    OE_ASSERT(!(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE), "Scene is incomplete");
    OE_ASSERT(scene->mRootNode != nullptr, "Scene has no root node");

    if (!ProcessNode(scene->mRootNode, scene)) {
      OE_ERROR("Failed to process root node : {}", path);
      return false;
    }

    return true;
  }

  bool ModelSerializer::ProcessNode(const aiNode* node, const aiScene* scene) {
    // process all the node's meshes (if any)
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
      aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
      if (mesh == nullptr) {
        OE_ERROR("Failed to get mesh : {}", node->mMeshes[i]);
        return false;
      }

      if (!ProcessMesh(mesh, scene)) {
        OE_ERROR("Failed to process mesh : {}", mesh->mName.C_Str());
        return false;
      }
    }

    // then do the same for each of its children
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
      if (!ProcessNode(node->mChildren[i], scene)) {
        OE_ERROR("Failed to process child node : {}", node->mChildren[i]->mName.C_Str());
        return false;
      }
    }

    return true;
  }

  bool ModelSerializer::ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
    OE_ASSERT(scene != nullptr, "Attempting to process a mesh without a scene");
    OE_ASSERT(mesh != nullptr, "Attempting to process a null mesh");

    // process mesh
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    // std::vector<Texture> textures;

    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
      Vertex& v = vertices.emplace_back();
      v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
      if (mesh->HasNormals()) {
        v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
      } else {
        v.normal = { 0.f, 0.f, 0.f };
      }

      if (mesh->mTextureCoords[0]) {
        v.uv_coord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
        v.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
      } else {
        v.uv_coord = { 0.f, 0.f };
        v.tangent = { 0.f, 0.f, 0.f };
        v.bitangent = { 0.f, 0.f, 0.f };
      }
    }

    Opt<uint32_t> num_index_per_face = std::nullopt;
    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];

      if (!num_index_per_face.has_value()) {
        num_index_per_face = face.mNumIndices;
      }

      for (uint32_t i = 0; i < face.mNumIndices; ++i) {
        if (index_stack.size() == 3) {
          Index& idx = indices.emplace_back();
          idx.v1 = index_stack.top();
          index_stack.pop();

          idx.v2 = index_stack.top();
          index_stack.pop();

          idx.v3 = index_stack.top();
          index_stack.pop();
        }

        index_stack.push(face.mIndices[i]);
      }
    }

    OE_ASSERT(num_index_per_face.has_value(), "Failed to get number of indices per face");
    models.push_back({
      .name = mesh->mName.C_Str(),
      .vertices = vertices,
      .triangles = indices,
      .vert_cnt = mesh->mNumVertices,
    });

    // process material
    // if (mesh->mMaterialIndex >= 0) {
    // }

    return true;
  }

}  // namespace other
