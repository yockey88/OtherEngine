/**
 * \file rendering/model.cpp
 **/
#include "rendering/model.hpp"

#include <glad/glad.h>

#include "asset/asset_defines.hpp"
#include "asset/asset_manager.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

  ModelSource::ModelSource(const std::vector<float>& vertices, const std::vector<uint32_t>& indices, const Layout& layout)
      : raw_vertices(vertices), raw_indices(indices), layout(layout) {
    raw_layout = layout.GetRawLayout();

    vertex_buffer = NewRef<VertexBuffer>(raw_vertices.data(), raw_vertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(raw_indices.data(), raw_indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    source_vao = NewRef<VertexArray>(raw_vertices, raw_indices, raw_layout);

    OE_ASSERT(vertex_buffer != nullptr, "null vertex buffer after model creation!");
    OE_ASSERT(source_vao != nullptr, "null vertex array after model creation!");
  }

  ModelSource::ModelSource(const std::vector<Vertex>& vertices, const std::vector<Index>& indices, const glm::mat4& transform)
      : vertices(vertices), indices(indices) {
    BuildVertexBuffer(vertices);
    BuildIndexBuffer(indices);

    vertex_buffer = NewRef<VertexBuffer>(raw_vertices.data(), raw_vertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(raw_indices.data(), raw_indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    layout = Vertex::Layout();
    raw_layout = Vertex::RawLayout();

    source_vao = NewRef<VertexArray>(raw_vertices, raw_indices, raw_layout);

    OE_ASSERT(vertex_buffer != nullptr, "null vertex buffer after model creation!");
    OE_ASSERT(source_vao != nullptr, "null vertex array after model creation!");
  }

  ModelSource::ModelSource(const std::vector<Vertex>& verts, const std::vector<Index>& idxs, const std::vector<SubMesh>& sms)
      : submeshes(sms), vertices(verts), indices(idxs) {
    BuildVertexBuffer(vertices);
    BuildIndexBuffer(indices);

    vertex_buffer = NewRef<VertexBuffer>(raw_vertices.data(), raw_vertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(raw_indices.data(), raw_indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    layout = Vertex::Layout();
    raw_layout = Vertex::RawLayout();

    source_vao = NewRef<VertexArray>(raw_vertices, raw_indices, raw_layout);

    OE_ASSERT(vertex_buffer != nullptr, "null vertex buffer after model creation!");
    OE_ASSERT(source_vao != nullptr, "null vertex array after model creation!");
  }

  std::vector<SubMesh>& ModelSource::SubMeshes() {
    return submeshes;
  }

  const std::vector<SubMesh>& ModelSource::SubMeshes() const {
    return submeshes;
  }

  Ref<Model> ModelSource::CreateModel(Ref<ModelSource>& source, const std::vector<uint32_t>& sub_meshes) {
    const AssetMetadata& meta = AppState::Assets()->GetMetadata(source->handle);
    AssetHandle handle = AssetManager::CreateMemOnly<Model>(fmtstr("{}.{}", meta.path, source->models_produced), source, sub_meshes);
    ++source->models_produced;
    return AssetManager::GetAsset<Model>(handle);
  }

  void ModelSource::DumpVertexBuffer() {
  }

  void ModelSource::BindVertexBuffer() {
    OE_ASSERT(vertex_buffer != nullptr, "trying to bind null vertex buffer after model creation!");
    vertex_buffer->Bind();
    CHECKGL();
  }

  void ModelSource::BindIndexBuffer() {
    index_buffer->Bind();
  }

  void ModelSource::DrawMesh(DrawMode mode) {
    CHECKGL();

    if (index_buffer != nullptr) {
      glDrawElements(mode, indices.size(), GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mode, 0, vertices.size() / Vertex::Stride());
    }

    CHECKGL();
  }

  void ModelSource::UnbindVertexBuffer() {
    vertex_buffer->Unbind();
  }

  void ModelSource::UnbindIndexBuffer() {
    index_buffer->Bind();
  }

  const std::vector<Vertex>& ModelSource::Vertices() const {
    return vertices;
  }

  const std::vector<float>& ModelSource::RawVertices() const {
    return raw_vertices;
  }

  const std::vector<Index>& ModelSource::Indices() const {
    return indices;
  }

  const std::vector<uint32_t>& ModelSource::RawIndices() const {
    return raw_indices;
  }

  const std::unordered_map<uint32_t, std::vector<Triangle>>& ModelSource::Triangles() const {
    return triangles;
  }

  const std::vector<Triangle>& ModelSource::Triangles(uint32_t sub_mesh_id) const {
    OE_ASSERT(triangles.find(sub_mesh_id) != triangles.end(), "Submesh not found in triangles map!");
    return triangles.at(sub_mesh_id);
  }

  const std::vector<uint32_t>& ModelSource::RawLayout() const {
    return raw_layout;
  }

  const Layout& ModelSource::GetLayout() const {
    return layout;
  }

  void ModelSource::BuildVertexBuffer(const std::vector<Vertex>& verts) {
    for (auto& v : verts) {
      raw_vertices.push_back(v.position.x);
      raw_vertices.push_back(v.position.y);
      raw_vertices.push_back(v.position.z);

      raw_vertices.push_back(v.normal.x);
      raw_vertices.push_back(v.normal.y);
      raw_vertices.push_back(v.normal.z);

      raw_vertices.push_back(v.tangent.x);
      raw_vertices.push_back(v.tangent.y);
      raw_vertices.push_back(v.tangent.z);

      raw_vertices.push_back(v.bitangent.x);
      raw_vertices.push_back(v.bitangent.y);
      raw_vertices.push_back(v.bitangent.z);

      raw_vertices.push_back(v.uv_coord.x);
      raw_vertices.push_back(v.uv_coord.y);
    }
  }

  void ModelSource::BuildIndexBuffer(const std::vector<Index>& vertices) {
    for (auto& idx : vertices) {
      raw_indices.push_back(idx.v1);
      raw_indices.push_back(idx.v2);
      raw_indices.push_back(idx.v3);
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
      glVertexAttribPointer(i, raw_layout[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));

      offset += raw_layout[i];
    }
  }

  Model::Model(Ref<ModelSource>& mesh_src)
      : model_source(mesh_src) {
    OE_ASSERT(mesh_src != nullptr, "Attempting construct model from null source!");
    SetSubMeshes({});
  }

  Model::Model(Ref<ModelSource>& model_src, const std::vector<uint32_t>& sub_meshes)
      : model_source(model_src) {
    OE_ASSERT(model_src != nullptr, "Attempting construct model from null source!");
    SetSubMeshes(sub_meshes);
  }

  Model::Model(const Ref<Model>& other) {
    handle = other->handle;
    model_source = Ref<ModelSource>::Clone(other->model_source);
    sub_meshes = other->sub_meshes;
    source_vao = Ref<VertexArray>::Clone(other->source_vao);
  }

  const std::vector<uint32_t>& Model::SubMeshes() const {
    return sub_meshes;
  }

  void Model::SetSubMeshes(const std::vector<uint32_t>& sms) {
    if (sms.empty()) {
      sub_meshes.resize(model_source->SubMeshes().size());
      for (uint32_t i = 0; i < sub_meshes.size(); ++i) {
        sub_meshes[i] = i;
      }
    } else {
      for (const uint32_t smidx : sms) {
        OE_ASSERT(smidx < model_source->SubMeshes().size(), "Submesh index out of bounds!");
      }

      sub_meshes = sms;
    }
  }

  Ref<ModelSource> Model::GetModelSource() const {
    OE_ASSERT(model_source != nullptr, "Model has null source!");
    return Ref<ModelSource>::Clone(model_source);
  }

  StaticModel::StaticModel(Ref<ModelSource>& model_src) {
    OE_ASSERT(model_src != nullptr, "Attempting construct model from null source!");
    model_source = Ref<ModelSource>::Clone(model_src);
    RebuildMesh();
  }

  StaticModel::StaticModel(const Ref<StaticModel>& other) {
    handle = other->handle;
    model_source = Ref<ModelSource>::Clone(other->model_source);
    model_vao = Ref<VertexArray>::Clone(other->model_vao);
  }

  void StaticModel::RebuildMesh() {
    OE_ASSERT(model_source != nullptr, "Static Model has null source!");

    const std::vector<float>& vertices = model_source->RawVertices();
    const std::vector<Index>& indices = model_source->Indices();
    std::vector<uint32_t> idxs{};

    for (const auto& i : indices) {
      idxs.push_back(i.v1);
      idxs.push_back(i.v2);
      idxs.push_back(i.v3);
    }
    model_vao = NewRef<VertexArray>(vertices, idxs);
    OE_ASSERT(model_vao != nullptr, "null model vao after model creation!");
  }

  Ref<ModelSource> StaticModel::GetModelSource() const {
    OE_ASSERT(model_source != nullptr, "Static Model has null source!");
    return Ref<ModelSource>::Clone(model_source);
  }

}  // namespace other
