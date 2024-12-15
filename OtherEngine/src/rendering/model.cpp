/**
 * \file rendering/model.cpp
 **/
#include "rendering/model.hpp"

#include <glad/glad.h>

#include "asset/asset_defines.hpp"
#include "asset/asset_manager.hpp"

#include "rendering/rendering_defines.hpp"

namespace other {

  ModelSource::ModelSource(std::vector<float>& vertices, std::vector<uint32_t>& indices, Layout& topology) {
    fvertices.swap(vertices);
    layout = topology;
    raw_layout = layout.GetRawLayout();
    raw_indices.swap(indices);

    vertex_buffer = NewRef<VertexBuffer>(fvertices.data(), fvertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data(), 3 * indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    OE_ASSERT(vertex_buffer != nullptr, "null vertex buffer after model creation!");
  }

  ModelSource::ModelSource(std::vector<float>& vertices, std::vector<Index>& idxs, Layout& layout) {
    SubMesh submesh;
    submesh.base_vertex = 0;
    submesh.base_idx = 0;
    submesh.idx_cnt = static_cast<uint32_t>(idxs.size());
    submesh.transform = glm::mat4(1.f);
    submeshes.push_back(submesh);

    fvertices.swap(vertices);
    raw_layout = layout.GetRawLayout();
    this->layout = layout;

    for (auto& idx : idxs) {
      raw_indices.push_back(idx.v1);
      raw_indices.push_back(idx.v2);
      raw_indices.push_back(idx.v3);
    }

    indices.swap(idxs);

    vertex_buffer = NewRef<VertexBuffer>(fvertices.data(), fvertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data(), 3 * indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& idxs, const glm::mat4& transform) {
    SubMesh submesh;
    submesh.base_vertex = 0;
    submesh.base_idx = 0;
    submesh.idx_cnt = static_cast<uint32_t>(indices.size());
    submesh.transform = transform;
    submeshes.push_back(submesh);

    BuildVertexBuffer(verts);
    indices.swap(idxs);

    vertex_buffer = NewRef<VertexBuffer>(fvertices.data(), fvertices.size() * sizeof(float));
    if (indices.size() > 0) {
      index_buffer = NewRef<VertexBuffer>(indices.data(), 3 * indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    raw_layout = Vertex::RawLayout();
    layout = Vertex::Layout();

    for (auto& idx : idxs) {
      raw_indices.push_back(idx.v1);
      raw_indices.push_back(idx.v2);
      raw_indices.push_back(idx.v3);
    }

    OE_ASSERT(vertex_buffer != nullptr, "null vertex buffer after model creation!");
  }

  ModelSource::ModelSource(std::vector<Vertex>& verts, std::vector<Index>& idxs, std::vector<SubMesh>& sms) {
    /// set submeshes
    submeshes.swap(sms);

    BuildVertexBuffer(verts);
    indices.swap(idxs);

    vertex_buffer = NewRef<VertexBuffer>(fvertices.data(), fvertices.size() * sizeof(float));
    if (indices.size() > 0) {
      /// indices stored as triangles with idx1 , idx2 , idx3
      index_buffer = NewRef<VertexBuffer>(indices.data(), 3 * indices.size() * sizeof(uint32_t), STATIC_DRAW, ELEMENT_ARRAY_BUFFER);
    }

    raw_layout = Vertex::RawLayout();
    layout = Vertex::Layout();

    for (auto& idx : idxs) {
      raw_indices.push_back(idx.v1);
      raw_indices.push_back(idx.v2);
      raw_indices.push_back(idx.v3);
    }

    OE_ASSERT(vertex_buffer != nullptr, "null vertex buffer after model creation!");
  }

  std::vector<SubMesh>& ModelSource::SubMeshes() {
    return submeshes;
  }

  const std::vector<SubMesh>& ModelSource::SubMeshes() const {
    return submeshes;
  }

  Ref<Model> ModelSource::CreateModel(Ref<ModelSource>& source, const std::vector<uint32_t>& sub_meshes) {
    const AssetMetadata& meta = AppState::Assets()->GetMetadata(source->handle);
    AssetHandle handle = AssetManager::CreateMemOnly<Model>(fmtstr("{}.{}", meta.path, source->models_produced), source);
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
      glVertexAttribPointer(i, raw_layout[i], GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)(offset * sizeof(float)));

      offset += raw_layout[i];
    }
  }

  Model::Model(Ref<ModelSource>& model_source)
      : model_source(model_source) {
    OE_ASSERT(model_source != nullptr, "Attempting construct model from null source!");
    SetSubMeshes({});
    RebuildMesh();

    /// build materials
  }

  Model::Model(Ref<ModelSource>& model_src, const std::vector<uint32_t>& sub_meshes)
      : model_source(model_src) {
    OE_ASSERT(model_src != nullptr, "Attempting construct model from null source!");
    SetSubMeshes(sub_meshes);
    RebuildMesh();

    /// build materials
  }

  Model::Model(const Ref<Model>& other) {
    handle = other->handle;
    model_source = Ref<ModelSource>::Clone(other->model_source);
    sub_meshes = other->sub_meshes;
    model_vao = other->model_vao;
  }

  const std::vector<uint32_t>& Model::SubMeshes() const {
    return sub_meshes;
  }

  void Model::SetSubMeshes(const std::vector<uint32_t>& sms) {
    if (!sms.empty()) {
      sub_meshes = sms;
    } else {
      sub_meshes = {};
    }
  }

  void Model::RebuildMesh() {
    OE_ASSERT(model_source != nullptr, "Model has null source!");
    if (model_vao != nullptr) {
      return;
    }

    auto& src_verts = model_source->RawVertices();
    auto& idxs = model_source->Indices();

    std::vector<uint32_t> raw_indices{};
    std::vector<float> raw_vertices{};

    if (!sub_meshes.empty()) {
      const std::vector<SubMesh>& src_submeshes = model_source->SubMeshes();

      for (const uint32_t sm_idx : sub_meshes) {
        const SubMesh& sm = src_submeshes[sm_idx];

        for (uint32_t i = sm.base_vertex; i < sm.base_vertex + sm.vert_cnt; ++i) {
          raw_vertices.push_back(src_verts[i]);
        }
        for (uint32_t i = sm.base_idx; i < sm.base_idx + sm.idx_cnt; ++i) {
          raw_indices.push_back(idxs[i].v1);
          raw_indices.push_back(idxs[i].v2);
          raw_indices.push_back(idxs[i].v3);
        }
      }
    } else {
      for (const auto& v : src_verts) {
        raw_vertices.push_back(v);
      }
      for (const auto& i : idxs) {
        raw_indices.push_back(i.v1);
        raw_indices.push_back(i.v2);
        raw_indices.push_back(i.v3);
      }
    }

    model_vao = NewRef<VertexArray>(raw_vertices, raw_indices);
    OE_ASSERT(model_vao != nullptr, "null model vao after model creation!");
  }

  Ref<ModelSource> Model::GetModelSource() const {
    OE_ASSERT(model_source != nullptr, "Model has null source!");
    return Ref<ModelSource>::Clone(model_source);
  }

  StaticModel::StaticModel(Ref<ModelSource>& model_source) {
    OE_ASSERT(model_source != nullptr, "Attempting construct model from null source!");
    this->model_source = Ref<ModelSource>::Clone(model_source);
    SetSubMeshes({});
    RebuildMesh();

    /// build materials
  }

  StaticModel::StaticModel(Ref<ModelSource>& model_src, const std::vector<uint32_t>& sub_meshes) {
    OE_ASSERT(model_src != nullptr, "Attempting construct model from null source!");
    this->model_source = Ref<ModelSource>::Clone(model_source);
    SetSubMeshes(sub_meshes);
    RebuildMesh();

    /// build materials
  }

  StaticModel::StaticModel(const Ref<StaticModel>& other) {
    handle = other->handle;
    model_source = Ref<ModelSource>::Clone(other->model_source);
    sub_meshes = other->sub_meshes;
    model_vao = other->model_vao;
  }

  const std::vector<uint32_t>& StaticModel::SubMeshes() const {
    return sub_meshes;
  }

  void StaticModel::SetSubMeshes(const std::vector<uint32_t>& sms) {
    if (!sms.empty()) {
      sub_meshes = sms;
    } else {
      sub_meshes = {};
    }
  }

  void StaticModel::RebuildMesh() {
    OE_ASSERT(model_source != nullptr, "Static Model has null source!");
    if (model_vao != nullptr) {
      return;
    }

    const std::vector<float>& src_vertices = model_source->RawVertices();
    const std::vector<Index>& src_indices = model_source->Indices();
    const std::vector<uint32_t>& src_raw_indices = model_source->RawIndices();
    const std::vector<uint32_t>& src_layout = model_source->RawLayout();

    std::vector<float> raw_vertices{};
    std::vector<uint32_t> raw_indices{};

    if (!sub_meshes.empty()) {
      const std::vector<SubMesh>& src_submeshes = model_source->SubMeshes();

      for (const uint32_t sm_idx : sub_meshes) {
        const SubMesh& sm = src_submeshes[sm_idx];

        for (uint32_t i = sm.base_vertex; i < sm.base_vertex + sm.vert_cnt; ++i) {
          raw_vertices.push_back(src_vertices[i]);
        }

        for (uint32_t i = sm.base_idx; i < sm.base_idx + sm.idx_cnt; ++i) {
          raw_indices.push_back(src_indices[i].v1);
          raw_indices.push_back(src_indices[i].v2);
          raw_indices.push_back(src_indices[i].v3);
        }
      }
    } else {
      raw_vertices = src_vertices;
      for (const auto& i : src_indices) {
        raw_indices.push_back(i.v1);
        raw_indices.push_back(i.v2);
        raw_indices.push_back(i.v3);
      }
    }

    model_vao = NewRef<VertexArray>(src_vertices, src_raw_indices, src_layout);
    OE_ASSERT(model_vao != nullptr, "null model vao after model creation!");
  }

  Ref<ModelSource> StaticModel::GetModelSource() const {
    OE_ASSERT(model_source != nullptr, "Static Model has null source!");
    return Ref<ModelSource>::Clone(model_source);
  }

}  // namespace other
