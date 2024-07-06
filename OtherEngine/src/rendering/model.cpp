/**
 * \file rendering/model.cpp
 **/
#include "rendering/model.hpp"

#include "core/rand.hpp"

namespace other {

  ModelSource::ModelSource(std::vector<float>& verts , std::vector<Index>& idxs , const glm::mat4& transform , Layout& lo) {
    handle = Random::GenerateUUID();
    
    SubMesh submesh;
    submesh.base_vertex = 0;
    submesh.base_idx = 0;
    submesh.idx_cnt = static_cast<uint32_t>(indices.size());
    submesh.transform = transform;
    submeshes.push_back(submesh);

    vertices.swap(verts);
    indices.swap(idxs);

    raw_layout = Vertex::RawLayout();
    layout = Vertex::Layout();

    vertex_buffer = NewRef<VertexBuffer>(vertices.data() , vertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data() , vertices.size() * sizeof(uint32_t));
    }
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& idxs, const glm::mat4& transform) {
    handle = Random::GenerateUUID();

    SubMesh submesh;
    submesh.base_vertex = 0;
    submesh.base_idx = 0;
    submesh.idx_cnt = static_cast<uint32_t>(indices.size());
    submesh.transform = transform;
    submeshes.push_back(submesh);

    BuildVertexBuffer(verts);
    indices.swap(idxs);

    vertex_buffer = NewRef<VertexBuffer>(vertices.data() , vertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data() , vertices.size() * sizeof(uint32_t));
    }

    raw_layout = Vertex::RawLayout();
    layout = Vertex::Layout();
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& idxs, std::vector<SubMesh>& submeshes) {
    /// set submeshes
    handle = Random::GenerateUUID();

    this->submeshes.swap(submeshes);
   
    BuildVertexBuffer(verts);
    indices.swap(idxs);

    vertex_buffer = NewRef<VertexBuffer>(vertices.data() , vertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data() , vertices.size() * sizeof(uint32_t));
    }

    raw_layout = Vertex::RawLayout();
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
      
  void ModelSource::BindVertexBuffer() {
    vertex_buffer->Bind();
  }

  void ModelSource::BindIndexBuffer() {
    index_buffer->Bind();
  }
  
  void ModelSource::UnbindVertexBuffer() {
    vertex_buffer->Unbind();
  }

  void ModelSource::UnbindIndexBuffer() {
    index_buffer->Bind();
  }

  const std::vector<float>& ModelSource::Vertices() const {
    return vertices;
  }

  const std::vector<Index>& ModelSource::Indices() const {
    return indices;
  }

  const std::vector<uint32_t>& ModelSource::RawLayout() const {
    return raw_layout;
  }

  const Layout& ModelSource::GetLayout() const {
    return layout;
  }
      
  void ModelSource::BuildVertexBuffer(const std::vector<Vertex>& verts) {
    for (auto& v : verts) {
      vertices.push_back(v.position.x);
      vertices.push_back(v.position.y);
      vertices.push_back(v.position.z);
       
      vertices.push_back(v.normal.x);
      vertices.push_back(v.normal.y);
      vertices.push_back(v.normal.z);
       
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
    handle = Random::GenerateUUID();
    SetSubMeshes({});

    /// build materials
  }

  Model::Model(Ref<ModelSource> model_src , const std::vector<uint32_t>& sub_meshes) 
      : model_source(model_src) {
    handle = Random::GenerateUUID();
    SetSubMeshes(sub_meshes);

    /// build materials
  }

  Model::Model(const Ref<Model>& other) 
      : model_source(other->model_source) /* materials */ {
    handle = Random::GenerateUUID();
    SetSubMeshes(other->sub_meshes);
  }
  
  const std::vector<uint32_t>& Model::SubMeshes() const {
    return sub_meshes;
  }

  void Model::SetSubMeshes(const std::vector<uint32_t>& sms) {
    if (!sms.empty()) {
      sub_meshes = sms;
    } else {
      const auto& src_submeshes = model_source->SubMeshes();
      sub_meshes.resize(src_submeshes.size());
      for (uint32_t i = 0; i < sub_meshes.size(); ++i) {
        sub_meshes[i] = i;
      }
    }
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
  
  StaticModel::StaticModel(Ref<ModelSource> model_source) {
    handle = Random::GenerateUUID();
	this->model_source = Ref<ModelSource>::Clone(model_source);
    SetSubMeshes({});

    /// build materials
  }

  StaticModel::StaticModel(Ref<ModelSource> model_src , const std::vector<uint32_t>& sub_meshes) {
    handle = Random::GenerateUUID();
    this->model_source = Ref<ModelSource>::Clone(model_source);
    SetSubMeshes(sub_meshes);

    /// build materials
  }

  StaticModel::StaticModel(const Ref<StaticModel>& other) 
      : model_source(other->model_source) /* materials */ {
    SetSubMeshes(other->sub_meshes);
    handle = Random::GenerateUUID();
  }
  
  const std::vector<uint32_t>& StaticModel::SubMeshes() const {
    return sub_meshes;
  }

  void StaticModel::SetSubMeshes(const std::vector<uint32_t>& sms) {
    if (!sms.empty()) {
      sub_meshes = sms;
    } else {
      const auto& src_submeshes = model_source->SubMeshes();
      OE_DEBUG("Submeshes for model source = {}" , model_source->SubMeshes().size());
      for (uint32_t i = 0; i < src_submeshes.size(); ++i) {
        sub_meshes.push_back(i);
      }
    }

    OE_DEBUG("Model {} has {} submeshes" , handle , sub_meshes.size());
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
