/**
 * \file rendering/model.cpp
 **/
#include "rendering/model.hpp"

#include <glad/glad.h>

#include "core/rand.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {

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

    vertex_buffer = NewRef<VertexBuffer>(fvertices.data() , fvertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data() , 3 * indices.size() * sizeof(uint32_t) ,
                                          STATIC_DRAW , ELEMENT_ARRAY_BUFFER);
    }

    raw_layout = Vertex::RawLayout();
    layout = Vertex::Layout();
    
    for (auto& idx : idxs) {
      raw_indices.push_back(idx.v1);
      raw_indices.push_back(idx.v2);
      raw_indices.push_back(idx.v3);
    }
    
    OE_ASSERT(vertex_buffer != nullptr , "null vertex buffer after model creation!");
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& idxs, std::vector<SubMesh>& submeshes) {
    /// set submeshes
    handle = Random::GenerateUUID();

    this->submeshes.swap(submeshes);
   
    BuildVertexBuffer(verts);
    indices.swap(idxs);

    vertex_buffer = NewRef<VertexBuffer>(fvertices.data() , fvertices.size() * sizeof(float));
    if (indices.size() > 0) {
      /// indices stored as triangles with idx1 , idx2 , idx3
      index_buffer = NewRef<VertexBuffer>(indices.data() , 3 * indices.size() * sizeof(uint32_t) ,
                                          STATIC_DRAW , ELEMENT_ARRAY_BUFFER);
    }

    raw_layout = Vertex::RawLayout();
    layout = Vertex::Layout();

    for (auto& idx : idxs) {
      raw_indices.push_back(idx.v1);
      raw_indices.push_back(idx.v2);
      raw_indices.push_back(idx.v3);
    }

    OE_ASSERT(vertex_buffer != nullptr , "null vertex buffer after model creation!");
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
    OE_ASSERT(vertex_buffer != nullptr , "trying to bind null vertex buffer after model creation!");
    vertex_buffer->Bind();
    CHECKGL();
  }

  void ModelSource::BindIndexBuffer() {
    index_buffer->Bind();
  }
      
  void ModelSource::DrawMesh(DrawMode mode) {
    CHECKGL();

    if (index_buffer != nullptr) {
      glDrawElements(mode , indices.size() , GL_UNSIGNED_INT , 0);
    } else {
      glDrawArrays(mode , 0 , vertices.size() / Vertex::Stride()); 
    }
    
    CHECKGL();
  }
  
  void ModelSource::UnbindVertexBuffer() {
    vertex_buffer->Unbind();
  }

  void ModelSource::UnbindIndexBuffer() {
    index_buffer->Bind();
  }
      
  const std::vector<float>& ModelSource::RawVertices() const {
    return fvertices;
  }
      
  const std::vector<uint32_t>& ModelSource::RawIndices() const {
    return raw_indices;
  }

  const std::vector<Vertex>& ModelSource::Vertices() const {
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
      fvertices.push_back(v.position.x);
      fvertices.push_back(v.position.y);
      fvertices.push_back(v.position.z);
       
      fvertices.push_back(v.normal.x);
      fvertices.push_back(v.normal.y);
      fvertices.push_back(v.normal.z);
       
      fvertices.push_back(v.tangent.x);
      fvertices.push_back(v.tangent.y);
      fvertices.push_back(v.tangent.z);
      
      fvertices.push_back(v.bitangent.x);
      fvertices.push_back(v.bitangent.y);
      fvertices.push_back(v.bitangent.z);
      
      fvertices.push_back(v.uv_coord.x);
      fvertices.push_back(v.uv_coord.y);
    }
  }

  void ModelSource::SetLayout() {
    uint32_t stride = 0;
    for (uint32_t i = 0; i < raw_layout.size(); i++) {
      stride += raw_layout[i];
    }
  
    uint32_t offset = 0;
    for (uint32_t i = 0; i < raw_layout.size(); i++) {
      glEnableVertexAttribArray(i);
      glVertexAttribPointer(i , raw_layout[i] , GL_FLOAT , GL_FALSE , stride * sizeof(float) , (void*)(offset * sizeof(float)));
  
      offset += raw_layout[i];
    }
  }

  Model::Model(Ref<ModelSource>& model_source) 
      : model_source(model_source) {
    OE_ASSERT(model_source != nullptr , "Attempting construct model from null source!");
    handle = Random::GenerateUUID();
    SetSubMeshes({});
    RebuildMesh();

    /// build materials
  }

  Model::Model(Ref<ModelSource>& model_src , const std::vector<uint32_t>& sub_meshes) 
      : model_source(model_src) {
    OE_ASSERT(model_src != nullptr , "Attempting construct model from null source!");
    handle = Random::GenerateUUID();
    SetSubMeshes(sub_meshes);
    RebuildMesh();

    /// build materials
  }

  Model::Model(const Ref<Model>& other) {
    handle = other->handle;
    model_source = Ref<ModelSource>::Clone(other->model_source);
    SetSubMeshes({});
    RebuildMesh();
  }
  
  Ref<VertexArray> Model::GetMesh() const {
    return Ref<VertexArray>::Clone(model_vao); 
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
 
  void Model::RebuildMesh() {
    auto& raw_vertices = model_source->RawVertices();
    auto& idxs = model_source->Indices();

    std::vector<uint32_t> raw_indices{};
    for (const auto& i : idxs) {
      raw_indices.push_back(i.v1);
      raw_indices.push_back(i.v2);
      raw_indices.push_back(i.v3);
    }

    model_vao = NewRef<VertexArray>(raw_vertices , raw_indices);
    OE_ASSERT(model_vao != nullptr , "null model vao after model creation!");
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
  
  StaticModel::StaticModel(Ref<ModelSource>& model_source) {
    OE_ASSERT(model_source != nullptr , "Attempting construct model from null source!");
    handle = Random::GenerateUUID();
    this->model_source = Ref<ModelSource>::Clone(model_source);
    SetSubMeshes({});
    RebuildMesh();

    /// build materials
  }

  StaticModel::StaticModel(Ref<ModelSource>& model_src , const std::vector<uint32_t>& sub_meshes) {
    OE_ASSERT(model_src != nullptr , "Attempting construct model from null source!");
    handle = Random::GenerateUUID();
    this->model_source = Ref<ModelSource>::Clone(model_source);
    SetSubMeshes(sub_meshes);
    RebuildMesh();

    /// build materials
  }

  StaticModel::StaticModel(const Ref<StaticModel>& other) {
    handle = other->handle;
    model_source = Ref<ModelSource>::Clone(other->model_source);
    SetSubMeshes({});
    RebuildMesh();
  }

  Ref<VertexArray> StaticModel::GetMesh() const {
    return Ref<VertexArray>::Clone(model_vao); 
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
     
  void StaticModel::RebuildMesh() {
    auto& raw_vertices = model_source->RawVertices();
    auto& raw_indices = model_source->RawIndices();

    model_vao = NewRef<VertexArray>(raw_vertices , raw_indices);
    OE_ASSERT(model_vao != nullptr , "null model vao after model creation!");
  }

  Ref<ModelSource> StaticModel::GetModelSource() {
    OE_ASSERT(model_source != nullptr , "Static Model has null source!");
    return model_source;
  }

  Ref<ModelSource> StaticModel::GetModelSource() const {
    return nullptr;
  }

  void StaticModel::SetModelAsset(Ref<ModelSource>& mesh_src) {
  }

} // namespace other
