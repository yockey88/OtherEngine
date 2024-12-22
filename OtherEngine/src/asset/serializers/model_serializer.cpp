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
#include "core/rand.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/model.hpp"

namespace other {

  void ModelSerializer::Serialize(const AssetMetadata& metadata) {}

  bool ModelSerializer::Load(AssetMetadata& metadata) {
    OE_DEBUG("Attempting to load model : {}", metadata.path);
    Assimp::Importer importer;

    uint32_t flags =
      aiProcess_CalcTangentSpace |          // Create binormals/tangents just in case
      aiProcess_Triangulate |               // Make sure we're triangles
      aiProcess_SortByPType |               // Split meshes by primitive type
      aiProcess_GenNormals |                // Make sure we have legit normals
      aiProcess_GenUVCoords |               // Convert UVs if required
                                            //		aiProcess_OptimizeGraph |
      aiProcess_RemoveRedundantMaterials |  // remove redundant materials
      aiProcess_FindDegenerates |           // remove degenerated polygons from the import
      aiProcess_FindInvalidData |           // detect invalid model data, such as invalid normal vectors
      aiProcess_TransformUVCoords |         // preprocess UV transformations (scaling, translation ...)
      aiProcess_FindInstances |             // search for instanced meshes and remove them by references to one master
      aiProcess_SplitByBoneCount |          // split meshes with too many bones. Necessary for our (limited) hardware skinning shader
      aiProcess_OptimizeMeshes |            // Batch draws where possible
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

    ProcessMaterials(scene);

    if (!ProcessNode(scene->mRootNode, scene)) {
      OE_ERROR("Failed to process root node : {}", metadata.path);
      return false;
    }

    /// normalize mesh scale to not be so big

    Ref<ModelSource> source = NewRef<ModelSource>(vertices, indices, submeshes);
    metadata.asset = source;
    return true;
  }

  namespace {

    glm::mat4 Mat4FromAIMatrix4x4(const aiMatrix4x4& matrix) {
      glm::mat4 result;
      // the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
      result[0][0] = matrix.a1;
      result[1][0] = matrix.a2;
      result[2][0] = matrix.a3;
      result[3][0] = matrix.a4;
      result[0][1] = matrix.b1;
      result[1][1] = matrix.b2;
      result[2][1] = matrix.b3;
      result[3][1] = matrix.b4;
      result[0][2] = matrix.c1;
      result[1][2] = matrix.c2;
      result[2][2] = matrix.c3;
      result[3][2] = matrix.c4;
      result[0][3] = matrix.d1;
      result[1][3] = matrix.d2;
      result[2][3] = matrix.d3;
      result[3][3] = matrix.d4;
      return result;
    }

  }  // anonymous namespace

  void ModelSerializer::ProcessMaterials(const aiScene* scene) {
    OE_ASSERT(scene != nullptr, "Attempting to build a material table without a scene");

    material_ids.resize(scene->mNumMaterials);
    for (uint32_t i = 0; i < scene->mNumMaterials; ++i) {
      const aiMaterial* material = scene->mMaterials[i];
      if (material == nullptr) {
        OE_ERROR("Failed to get material : {}", i);
        continue;
      }

      material_ids[i] = GetMaterial(i, scene);
    }
  }

  bool ModelSerializer::ProcessNode(const aiNode* node, const aiScene* scene, const uint32_t parent, const glm::mat4& parent_transform, uint32_t level) {
    OE_ASSERT(scene != nullptr, "Attempting to process a node without a scene");
    OE_ASSERT(node != nullptr, "Attempting to process a null node");

    uint32_t idx = (uint32_t)nodes.size();
    MeshNode& mesh_node = nodes.emplace_back();

    mesh_node.name = node->mName.C_Str();
    mesh_node.local_transform = Mat4FromAIMatrix4x4(node->mTransformation);
    mesh_node.parent = parent;

    if (parent != 0xFFFFFFFF) {
      nodes[parent].children.push_back(idx);
    }

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

    mesh_node.children.resize(node->mNumChildren);
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
      if (!ProcessNode(node->mChildren[i], scene, idx, mesh_node.local_transform, level + 1)) {
        OE_ERROR("Failed to process child node : {}", node->mChildren[i]->mName.C_Str());
        return false;
      }
    }

    for (const auto& submesh : submeshes) {
      BBox transformed_submesh_bounds = submesh.bounds;

      glm::vec3 min = glm::vec3(submesh.transform * glm::vec4(transformed_submesh_bounds.min, 1.0f));
      glm::vec3 max = glm::vec3(submesh.transform * glm::vec4(transformed_submesh_bounds.max, 1.0f));

      mesh_bounds.min.x = glm::min(mesh_bounds.min.x, min.x);
      mesh_bounds.min.y = glm::min(mesh_bounds.min.y, min.y);
      mesh_bounds.min.z = glm::min(mesh_bounds.min.z, min.z);

      mesh_bounds.max.x = glm::max(mesh_bounds.max.x, max.x);
      mesh_bounds.max.y = glm::max(mesh_bounds.max.y, max.y);
      mesh_bounds.max.z = glm::max(mesh_bounds.max.z, max.z);
    }

    /// process bones and stuff here

    // then do the same for each of its children

    return true;
  }

  bool ModelSerializer::ProcessMesh(const aiMesh* mesh, const aiScene* scene) {
    OE_ASSERT(scene != nullptr, "Attempting to process a mesh without a scene");
    OE_ASSERT(mesh != nullptr, "Attempting to process a null mesh");

    SubMesh& submesh = submeshes.emplace_back();
    submesh.sub_mesh_id = Random::GenerateUUID();

    submesh.model_name = mesh->mName.C_Str();
    submesh.base_vertex = vertex_offset;
    submesh.vert_cnt = mesh->mNumVertices;
    submesh.base_idx = index_offset;
    submesh.idx_cnt = mesh->mNumFaces * 3;
    submesh.material_id = material_ids[mesh->mMaterialIndex];

    BBox& bounds = submesh.bounds;
    bounds = BBox::empty;
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
      Vertex& v = vertices.emplace_back();
      v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
      submesh.bounds.min.x = glm::min(v.position.x, submesh.bounds.min.x);
      submesh.bounds.min.y = glm::min(v.position.y, submesh.bounds.min.y);
      submesh.bounds.min.z = glm::min(v.position.z, submesh.bounds.min.z);

      submesh.bounds.max.x = glm::max(v.position.x, submesh.bounds.max.x);
      submesh.bounds.max.y = glm::max(v.position.y, submesh.bounds.max.y);
      submesh.bounds.max.z = glm::max(v.position.z, submesh.bounds.max.z);

      if (mesh->HasNormals()) {
        v.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
      } else {
        v.normal = { 0.f, 0.f, 0.f };
      }

      if (mesh->HasTangentsAndBitangents()) {
        v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
        v.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
      } else {
        v.tangent = { 0.f, 0.f, 0.f };
        v.bitangent = { 0.f, 0.f, 0.f };
      }

      if (mesh->HasTextureCoords(0)) {
        v.uv_coord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        v.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
        v.bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
      } else {
        v.uv_coord = { 0.f, 0.f };
        v.tangent = { 0.f, 0.f, 0.f };
        v.bitangent = { 0.f, 0.f, 0.f };
      }

      vertex_offset += Vertex::Stride();
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
      aiFace face = mesh->mFaces[i];
      OE_ASSERT(face.mNumIndices == 3, "Other Engine does not support untriangulated meshes");
      Index& idx = indices.emplace_back();
      idx = { face.mIndices[0], face.mIndices[1], face.mIndices[2] };
    }

    return true;
  }

  UUID ModelSerializer::GetMaterial(const uint32_t idx, const aiScene* scene) {
    OE_ASSERT(scene != nullptr, "Scene is null");
    OE_ASSERT(idx < scene->mNumMaterials, "Material index out of bounds");

    aiMaterial* material = scene->mMaterials[idx];
    if (material == nullptr) {
      OE_ERROR("Failed to get material : {}", idx);
      return 0u;
    }

    aiString mat_name;
    material->Get(AI_MATKEY_NAME, mat_name);

    OE_DEBUG("  {0} (Index = {1})", mat_name.data, idx);
    aiString tex_path;
    uint32_t textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
    OE_DEBUG("    TextureCount = {0}", textureCount);

    Opt<glm::vec4> albedo_col = std::nullopt;
    Opt<float> emission = std::nullopt;

    aiColor3D col;
    aiColor3D emissive;
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, col) == AI_SUCCESS) {
      albedo_col = { col.r, col.g, col.b, 1.f };
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

    Opt<MaterialTable::Texture> albedo_tex = std::nullopt;
    Opt<MaterialTable::Texture> normal_tex = std::nullopt;
    Opt<MaterialTable::Texture> roughness_tex = std::nullopt;

    if (has_albedo_map) {
      if (const aiTexture* tex = scene->GetEmbeddedTexture(tex_path.C_Str())) {
        uint8_t* data = reinterpret_cast<uint8_t*>(tex->pcData);
        albedo_tex = MaterialTable::CreateTexture(4 * tex->mWidth * tex->mHeight, data, { tex->mWidth, tex->mHeight });
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
            albedo_tex = MaterialTable::CreateTexture(data.size(), data.data(), { 1920.f, 1080.f });
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
      albedo_tex = MaterialTable::CreateTexture(glm::vec4{ 1.f }, { 1920.f, 1080.f });
    } else if (albedo_col.has_value()) {
      albedo_tex = MaterialTable::CreateTexture(albedo_col.value(), { 1920.f, 1080.f });
    }

    // Normal maps
    bool has_normal_map = material->GetTexture(aiTextureType_NORMALS, 0, &tex_path) == AI_SUCCESS;
    fallback = !has_normal_map;
    if (has_normal_map) {
      if (const aiTexture* texture = scene->GetEmbeddedTexture(tex_path.C_Str())) {
        uint8_t* data = reinterpret_cast<uint8_t*>(texture->pcData);
        normal_tex = MaterialTable::CreateTexture(4 * texture->mWidth * texture->mHeight, data, { texture->mWidth, texture->mHeight });
      } else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
        Ref<FileHandle> file = Filesystem::GetFile(full_path);
        if (file != nullptr) {
          std::vector<uint8_t> data = file->ReadBytes();
          if (data.empty()) {
            OE_ERROR("Failed to get texture  from file : {}", full_path);
            fallback = true;
          } else {
            normal_tex = MaterialTable::CreateTexture(data.size(), data.data(), { 1920.f, 1080.f });
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
      normal_tex = MaterialTable::CreateTexture(glm::vec4{ 0.5f, 0.5f, 1.f, 1.f }, { 1920.f, 1080.f });
    }

    // Roughness map
    bool has_roughness_map = material->GetTexture(aiTextureType_SHININESS, 0, &tex_path) == AI_SUCCESS;
    fallback = !has_roughness_map;
    if (has_roughness_map) {
      if (const aiTexture* texture = scene->GetEmbeddedTexture(tex_path.C_Str())) {
        uint8_t* data = reinterpret_cast<uint8_t*>(texture->pcData);
        roughness_tex = MaterialTable::CreateTexture(4 * texture->mWidth * texture->mHeight, data, { texture->mWidth, texture->mHeight });
      } else if (Path full_path = std::filesystem::absolute(Path{ tex_path.C_Str() }); Filesystem::FileExists(full_path)) {
        Ref<FileHandle> file = Filesystem::GetFile(full_path);
        if (file != nullptr) {
          std::vector<uint8_t> data = file->ReadBytes();
          if (data.empty()) {
            OE_ERROR("Failed to get texture  from file : {}", full_path);
            fallback = true;
          } else {
            roughness_tex = MaterialTable::CreateTexture(data.size(), data.data(), { 1920.f, 1080.f });
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
      roughness_tex = MaterialTable::CreateTexture(glm::vec4{ 0.5f, 0.5f, 0.5f, 1.f }, { 1920.f, 1080.f });
    }

    OE_ASSERT(albedo_tex.has_value(), "Albedo texture is null");
    OE_ASSERT(normal_tex.has_value(), "Normal texture is null");
    OE_ASSERT(roughness_tex.has_value(), "Roughness texture is null");

    Ref<MaterialTable> material_table = AssetManager::GetMaterialTable();
    OE_ASSERT(material_table != nullptr, "Material table is null");

    UUID mat_id = material_table->RegisterMaterial(albedo_tex.value(), normal_tex.value(), roughness_tex.value());
    OE_DEBUG("    Material ID = {0}", mat_id);
    return mat_id;
  }

}  // namespace other
