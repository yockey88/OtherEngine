/**
 * \file rendering/model_factory.cpp
 **/
#include "rendering/model_factory.hpp"

#include "asset/asset_manager.hpp"

#include "rendering/vertex.hpp"
#include "rendering/model.hpp"

namespace other {

  AssetHandle ModelFactory::CreateTriangle(const glm::vec3& pos) {
    std::vector<Vertex> vertices{};
    vertices.resize(3);

    vertices[0].position = { pos.x - 0.5f , pos.y - 0.5f , pos.z };
    vertices[1].position = { pos.x + 0.5f , pos.y - 0.5f , pos.z };
    vertices[2].position = { pos.x        , pos.y + 0.5f , pos.z };
    
    vertices[0].normal = { -1.f , -1.f , 0.f };
    vertices[1].normal = {  1.f , -1.f , 0.f };
    vertices[2].normal = {  0.f ,  1.f , 0.f };

    std::vector<Index> indices;

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices , indices , glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(source);
    return handle;
  }
      
  AssetHandle ModelFactory::CreateRect(const glm::vec3& pos , const glm::vec2& half_extents) {
    std::vector<Vertex> vertices{};
    vertices.resize(6);

    vertices[0].position = { 0.5 + half_extents.x , 0.5 + half_extents.y , pos.z };
    vertices[1].position = { 0.5 + half_extents.x , 0.5 - half_extents.y , pos.z };
    vertices[2].position = { 0.5 - half_extents.x , 0.5 - half_extents.y , pos.z };
    vertices[3].position = { 0.5 - half_extents.x , 0.5 + half_extents.y , pos.z };
    
    vertices[0].normal = {  1.f ,  1.f , 0.f };
    vertices[1].normal = {  1.f , -1.f , 0.f };
    vertices[2].normal = { -1.f , -1.f , 0.f };
    vertices[3].normal = { -1.f ,  1.f , 0.f };

    std::vector<Index> indices;
    indices.resize(2);

    indices[0] = { 0 , 1 , 3 };
    indices[1] = { 1 , 2 , 3 };

    AssetHandle source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices , indices , glm::mat4(1.f));
    Ref<ModelSource> source = AssetManager::GetAsset<ModelSource>(source_handle);

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(source);
    return handle;
  }

  AssetHandle ModelFactory::CreateBox(const glm::vec3& size) {
    std::vector<Vertex> vertices;
    vertices.resize(8);

    vertices[0].position = { -size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f };
    vertices[1].position = {  size.x / 2.0f, -size.y / 2.0f,  size.z / 2.0f };
    vertices[2].position = {  size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f };
    vertices[3].position = { -size.x / 2.0f,  size.y / 2.0f,  size.z / 2.0f };
    vertices[4].position = { -size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f };
    vertices[5].position = {  size.x / 2.0f, -size.y / 2.0f, -size.z / 2.0f };
    vertices[6].position = {  size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f };
    vertices[7].position = { -size.x / 2.0f,  size.y / 2.0f, -size.z / 2.0f };
    
    vertices[0].normal = { -1.0f, -1.0f,  1.0f };
    vertices[1].normal = {  1.0f, -1.0f,  1.0f };
    vertices[2].normal = {  1.0f,  1.0f,  1.0f };
    vertices[3].normal = { -1.0f,  1.0f,  1.0f };
    vertices[4].normal = { -1.0f, -1.0f, -1.0f };
    vertices[5].normal = {  1.0f, -1.0f, -1.0f };
    vertices[6].normal = {  1.0f,  1.0f, -1.0f };
    vertices[7].normal = { -1.0f,  1.0f, -1.0f };
    
    std::vector<Index> indices;
    indices.resize(12);
    indices[0] =  { 0, 1, 2 };
    indices[1] =  { 2, 3, 0 };
    indices[2] =  { 1, 5, 6 };
    indices[3] =  { 6, 2, 1 };
    indices[4] =  { 7, 6, 5 };
    indices[5] =  { 5, 4, 7 };
    indices[6] =  { 4, 0, 3 };
    indices[7] =  { 3, 7, 4 };
    indices[8] =  { 4, 5, 1 };
    indices[9] =  { 1, 0, 4 };
    indices[10] = { 3, 2, 6 };
    indices[11] = { 6, 7, 3 };

    AssetHandle mesh_source_handle = AssetManager::CreateMemOnly<ModelSource>(vertices , indices , glm::mat4(1.f));
    Ref<ModelSource> mesh_source = AssetManager::GetAsset<ModelSource>(mesh_source_handle);

    AssetHandle handle = AssetManager::CreateMemOnly<StaticModel>(mesh_source);
    OE_DEBUG("Created cube mesh [{}]" , handle);
    return handle; 
  }
  
  AssetHandle ModelFactory::CreateSphere(float radius) {
    return 0;
  }

  AssetHandle ModelFactory::CreateCapsule(float radius , float height) {
    return 0;
  }

} // namespace other
