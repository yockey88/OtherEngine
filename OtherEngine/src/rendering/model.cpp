/**
 * \file rendering/model.cpp
 **/
#include "rendering/model.hpp"

namespace other {
  ModelSource::ModelSource(std::vector<float>& verts , std::vector<Index>& idxs , const glm::mat4& transform , 
                           std::vector<uint32_t>& layout) {
    vertices.swap(verts);
    indices.swap(idxs);
    layout.swap(layout);

    vertex_buffer = Ref<VertexBuffer>::Create(vertices.data() , vertices.size());
    index_buffer = Ref<VertexBuffer>::Create(indices.data() , indices.size());
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& idxs, const glm::mat4& transform) {
    SubMesh submesh;
    submesh.base_vertex = 0;
    submesh.base_idx = 0;
    submesh.idx_cnt = static_cast<uint32_t>(indices.size());
    submesh.transform = transform;
    sub_meshes.push_back(submesh);

    OE_DEBUG("Submeshes for model source : {}" , sub_meshes.size());

    BuildVertexBuffer(verts);

    vertex_buffer = Ref<VertexBuffer>::Create(vertices.data() , vertices.size());
    index_buffer = Ref<VertexBuffer>::Create(indices.data() , indices.size());

    layout = Vertex::Layout();
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& indices, std::vector<SubMesh>& submeshes) {
    /// set submeshes
   
    BuildVertexBuffer(verts);

    index_buffer = Ref<VertexBuffer>::Create(indices.data() , vertices.size());
    layout = Vertex::Layout();

  }

  std::vector<SubMesh>& ModelSource::SubMeshes() {
    return submeshes;
  }

  const std::vector<SubMesh>& ModelSource::SubMeshes() const {
    return submeshes;
  }

  void ModelSource::DumpVertexBuffer() {
  }

  const std::vector<float>& ModelSource::Vertices() const {
    return vertices;
  }

  const std::vector<Index>& ModelSource::Indices() const {
    return indices;
  }
      
  void ModelSource::BuildVertexBuffer(const std::vector<Vertex>& verts) {
    for (auto& v : verts) {
      vertices.push_back(v.position.x);
      vertices.push_back(v.position.y);
      vertices.push_back(v.position.z);
      
      vertices.push_back(v.color.x);
      vertices.push_back(v.color.y);
      vertices.push_back(v.color.z);
      
      vertices.push_back(v.normal.x);
      vertices.push_back(v.normal.y);
      vertices.push_back(v.normal.z);
      
      vertices.push_back(v.tangent.x);
      vertices.push_back(v.tangent.y);
      vertices.push_back(v.tangent.z);
      
      vertices.push_back(v.tangent.x);
      vertices.push_back(v.tangent.y);
      vertices.push_back(v.tangent.z);
      
      vertices.push_back(v.bitangent.x);
      vertices.push_back(v.bitangent.y);
      vertices.push_back(v.bitangent.z);
      
      vertices.push_back(v.uv_coord.x);
      vertices.push_back(v.uv_coord.y);
    }
  }

  Model::Model(Ref<ModelSource> model_source) 
      : model_source(model_source) {
    SetSubMeshes({});

    /// build materials
  }

  Model::Model(Ref<ModelSource> model_src , const std::vector<uint32_t>& sub_meshes) 
      : model_source(model_src) {
    SetSubMeshes(sub_meshes);

    /// build materials
  }

  Model::Model(const Ref<Model>& other) 
      : model_source(other->model_source) /* materials */ {
    SetSubMeshes(other->sub_meshes);
  }
  
  const std::vector<uint32_t>& Model::SubMeshes() const {
    return sub_meshes;
  }

  void Model::SetSubMeshes(const std::vector<uint32_t>& sms) {
    sub_meshes = sms;
  }

  Ref<ModelSource> Model::GetModelSource() {
    return Ref<ModelSource>::Clone(model_source);
  }

  Ref<ModelSource> Model::GetModelSource() const {
    return Ref<ModelSource>::Clone(model_source);
  }

  void Model::SetModelAsset(Ref<ModelSource> mesh_src) {
    /// idk
  }
  
  StaticModel::StaticModel(Ref<ModelSource> model_source) 
      : model_source(model_source) {
    SetSubMeshes({});

    /// build materials
  }

  StaticModel::StaticModel(Ref<ModelSource> model_src , const std::vector<uint32_t>& sub_meshes) 
      : model_source(model_src) {
    SetSubMeshes(sub_meshes);

    /// build materials
  }

  StaticModel::StaticModel(const Ref<StaticModel>& other) 
      : model_source(other->model_source) /* materials */ {
    SetSubMeshes(other->sub_meshes);
  }
  
  const std::vector<uint32_t>& StaticModel::SubMeshes() const {
    return sub_meshes;
  }

  void StaticModel::SetSubMeshes(const std::vector<uint32_t>& sms) {
    sub_meshes = sms;
  }

  Ref<ModelSource> StaticModel::GetModelSource() {
    return Ref<ModelSource>::Clone(model_source);
  }

  Ref<ModelSource> StaticModel::GetModelSource() const {
    return nullptr;
  }

  void StaticModel::SetModelAsset(Ref<ModelSource> mesh_src) {
    /// idk
  }

} // namespace other
