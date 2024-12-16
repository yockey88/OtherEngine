/**
 * \file asset/serializers/model_serializer.cpp
 **/
#include "asset/serializers/model_serializer.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "core/filesystem.hpp"
#include "core/logger.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/model.hpp"
#include "rendering/texture.hpp"

namespace other {

  void ModelSerializer::Serialize(const AssetMetadata& metadata) {}

  bool ModelSerializer::Load(AssetMetadata& metadata) {
    OE_DEBUG("Attempting to load model : {}", metadata.path);
    Assimp::Importer importer;

    /// others
    // aiProcess_RemoveRedundantMaterials |  // remove redundant materials
    // aiProcess_FindDegenerates |           // remove degenerated polygons from the import
    // aiProcess_FindInvalidData |           // detect invalid model data, such as invalid normal vectors
    // aiProcess_TransformUVCoords |         // preprocess UV transformations (scaling, translation ...)
    // aiProcess_FindInstances |             // search for instanced meshes and remove them by references to one master
    // aiProcess_SplitByBoneCount |          // split meshes with too many bones. Necessary for our (limited) hardware skinning shader

    uint32_t flags =
      aiProcess_CalcTangentSpace |  // Create binormals/tangents just in case
      aiProcess_Triangulate |       // Make sure we're triangles
      aiProcess_SortByPType |       // Split meshes by primitive type
      aiProcess_GenNormals |        // Make sure we have legit normals
      aiProcess_GenUVCoords |       // Convert UVs if required
                                    //		aiProcess_OptimizeGraph |
      aiProcess_OptimizeMeshes |    // Batch draws where possible
      aiProcess_JoinIdenticalVertices |
      aiProcess_LimitBoneWeights |       // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
      aiProcess_ValidateDataStructure |  // Validation
      aiProcess_GlobalScale;             // e.g. convert cm to m for fbx import (and other formats where cm is native)
    const aiScene* scene = importer.ReadFile(metadata.path.string(), flags);
    if (scene == nullptr) {
      OE_ERROR("Failed to load model : {}", metadata.path);
      return false;
    }

    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
      OE_ERROR("Failed to load model : {}\n[ASSIMP ERROR : {}]", metadata.path, importer.GetErrorString());
      return false;
    }

    // Ref<MaterialTable> mat_table = NewRef<MaterialTable>(1, models.size(), glm::vec2{ 1920.f, 1080.f });
    if (!ProcessScene(scene, metadata.path.string())) {
      OE_ERROR("Failed to process model scene : {}", metadata.path);
      return false;
    }

    std::vector<SubMesh> submeshes;
    std::vector<Vertex> source_vertices;
    std::vector<Index> source_indices;

    glm::vec3 max_bounds = glm::vec3(-infinity<float>());
    glm::vec3 min_bounds = glm::vec3(infinity<float>());

    uint32_t idx_offset = 0;
    uint32_t vert_offset = 0;

    for (uint32_t i = 0; i < models.size(); ++i) {
      ModelData& model = models[i];
      OE_TRACE(" > loaded model : {}", model.name);
      SubMesh& submesh = submeshes.emplace_back();

      submesh.base_vertex = vert_offset;
      submesh.base_idx = idx_offset;
      submesh.model_name = model.name;
      submesh.material = model.material;

      for (Vertex& v : model.vertices) {
        source_vertices.push_back(v);

        max_bounds = glm::max(max_bounds, v.position);
        min_bounds = glm::min(min_bounds, v.position);
      }

      for (Index& idx : model.triangles) {
        source_indices.push_back(idx);
      }
      submesh.idx_cnt = source_indices.size();
    }

    // BBox mesh_bounds = { min_bounds, max_bounds };
    /// scale mesh to scene size

    /// normalize mesh scale to not be so big

    Ref<ModelSource> source = NewRef<ModelSource>(source_vertices, source_indices, submeshes);
    source->SetMaterialTable(material_table);

    metadata.asset = source;
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

    if (!BuildMaterialTable(scene)) {
      OE_ERROR("Failed to build material table");
      return false;
    }

    /// nodes are processed, now process materials using stored models
    for (ModelData& model : models) {
      model.material = {
        .albedo_tex_idx = model.mat_idx,
        .normal_tex_idx = model.mat_idx,
        .roughness_tex_idx = model.mat_idx,
      };
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
      .mat_idx = mesh->mMaterialIndex,
    });

    return true;
  }

  bool ModelSerializer::BuildMaterialTable(const aiScene* scene) {
    OE_ASSERT(scene != nullptr, "Attempting to build a material table without a scene");

    uint32_t mip_levels = 1;
    material_table = NewRef<MaterialTable>(mip_levels, scene->mNumMaterials, glm::vec2{ 1920.f, 1080.f });

    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
      aiMaterial* material = scene->mMaterials[i];
      if (material == nullptr) {
        OE_ERROR("Failed to get material : {}", i);
        return false;
      }

      aiString mat_name;
      material->Get(AI_MATKEY_NAME, mat_name);

      OE_DEBUG("  {0} (Index = {1})", mat_name.data, i);
      aiString tex_path;
      uint32_t textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
      OE_DEBUG("    TextureCount = {0}", textureCount);

      Opt<glm::vec3> albedo_col = std::nullopt;
      Opt<float> emission = std::nullopt;

      aiColor3D col;
      aiColor3D emissive;
      if (material->Get(AI_MATKEY_COLOR_DIFFUSE, col) == AI_SUCCESS) {
        albedo_col = { col.r, col.g, col.b };
      }

      if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
        emission = emissive.r;
      }

      float roughness;
      float metalness;
      if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != aiReturn_SUCCESS) {
        roughness = 0.5f;  // Default value
      }

      if (material->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS) {
        metalness = 0.0f;
      }

      OE_DEBUG("    COLOR = {0}, {1}, {2}", col.r, col.g, col.b);
      OE_DEBUG("    ROUGHNESS = {0}", roughness);
      OE_DEBUG("    METALNESS = {0}", metalness);
      bool has_albedo_map = material->GetTexture(aiTextureType_DIFFUSE, 0, &tex_path) == AI_SUCCESS;
      bool fallback = !has_albedo_map;

      if (has_albedo_map) {
        if (const aiTexture* tex = scene->GetEmbeddedTexture(tex_path.C_Str())) {
          uint8_t* data = reinterpret_cast<uint8_t*>(tex->pcData);
          material_table->SetTexture(MaterialTable::ALBEDO, i, data);
        }
        /// if texture not embedded, attempt loading from file if it exists
        else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
          Ref<FileHandle> file = Filesystem::GetFile(full_path);
          if (file != nullptr) {
            std::vector<uint8_t> data = file->ReadBytes();
            if (data.empty()) {
              OE_ERROR("Failed to get texture  from file : {}", full_path);
              fallback = !albedo_col.has_value();
            } else {
              material_table->SetTexture(MaterialTable::ALBEDO, i, data.data());
            }
          } else {
            OE_ERROR("Failed to get file handle : {}", full_path);
            fallback = !albedo_col.has_value();
          }
        }
        /// file does not exist, fallback to color
        else {
          OE_DEBUG("    Could not load texture : {0}", tex_path.C_Str());
          fallback = !albedo_col.has_value();
        }
      }

      if (fallback) {
        material_table->SetTexture(MaterialTable::ALBEDO, i, glm::vec3{ 1.f, 1.f, 1.f });
      } else if (albedo_col.has_value()) {
        material_table->SetTexture(MaterialTable::ALBEDO, i, albedo_col.value());
      }

      // Normal maps
      bool has_normal_map = material->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == AI_SUCCESS;
      fallback = !has_normal_map;
      if (has_normal_map) {
        if (const aiTexture* texture = scene->GetEmbeddedTexture(tex_path.C_Str())) {
          uint8_t* data = reinterpret_cast<uint8_t*>(texture->pcData);
          material_table->SetTexture(MaterialTable::NORMAL, i, data);
        } else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
          Ref<FileHandle> file = Filesystem::GetFile(full_path);
          if (file != nullptr) {
            std::vector<uint8_t> data = file->ReadBytes();
            if (data.empty()) {
              OE_ERROR("Failed to get texture  from file : {}", full_path);
              fallback = true;
            } else {
              material_table->SetTexture(MaterialTable::NORMAL, i, data.data());
            }
          } else {
            OE_ERROR("Failed to get file handle : {}", full_path);
            fallback = true;
          }
        } else {
          OE_DEBUG("    Could not load texture : {0}", tex_path.C_Str());
          fallback = true;
        }
      }

      if (fallback) {
        material_table->SetTexture(MaterialTable::NORMAL, i, glm::vec3{ 1.f, 1.f, 1.f });
      }

      // Roughness map
      bool has_roughness_map = material->GetTexture(aiTextureType_SHININESS, 0, &tex_path) == AI_SUCCESS;
      fallback = !has_roughness_map;
      if (has_roughness_map) {
        if (const aiTexture* texture = scene->GetEmbeddedTexture(tex_path.C_Str())) {
          uint8_t* data = reinterpret_cast<uint8_t*>(texture->pcData);
          material_table->SetTexture(MaterialTable::ROUGHNESS, i, data);
        } else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
          Ref<FileHandle> file = Filesystem::GetFile(full_path);
          if (file != nullptr) {
            std::vector<uint8_t> data = file->ReadBytes();
            if (data.empty()) {
              OE_ERROR("Failed to get texture  from file : {}", full_path);
              fallback = true;
            } else {
              material_table->SetTexture(MaterialTable::ROUGHNESS, i, data.data());
            }
          } else {
            OE_ERROR("Failed to get file handle : {}", full_path);
            fallback = true;
          }
        } else {
          OE_DEBUG("    Could not load texture : {0}", tex_path.C_Str());
          fallback = true;
        }
      }

      if (fallback) {
        material_table->SetTexture(MaterialTable::ROUGHNESS, i, glm::vec3{ 1.f, 1.f, 1.f });
      }
    }

    return true;
  }

}  // namespace other
