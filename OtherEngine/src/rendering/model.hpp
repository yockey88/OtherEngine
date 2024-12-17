/**
 * \file rendering/model.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_HPP
#define OTHER_ENGINE_MODEL_HPP

#include <glm/glm.hpp>

#include "math/bounding_box.hpp"

#include "asset/asset.hpp"
#include "asset/asset_defines.hpp"

#include "rendering/material_table.hpp"
#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

  class Model;
  class StaticModel;

  class ModelSource : public Asset {
   public:
    OE_ASSET(MODEL_SOURCE);

    ModelSource() {}
    ModelSource(std::vector<float>& vertices, std::vector<uint32_t>& indices, Layout& layout);
    ModelSource(std::vector<float>& vertices, std::vector<Index>& indices, Layout& layout);
    ModelSource(std::vector<Vertex>& vertices, std::vector<Index>& indices, const glm::mat4& transform);
    ModelSource(std::vector<Vertex>& vertices, std::vector<Index>& indices, std::vector<SubMesh>& submeshes);

    virtual ~ModelSource() {}

    std::vector<SubMesh>& SubMeshes();
    const std::vector<SubMesh>& SubMeshes() const;

    static Ref<Model> CreateModel(Ref<ModelSource>& source, const std::vector<uint32_t>& sub_meshes);
    static Ref<StaticModel> CreateStaticModel(Ref<ModelSource>& source);

    void DumpVertexBuffer();

    void BindVertexBuffer();
    void BindIndexBuffer();

    void DrawMesh(DrawMode mode);

    void UnbindVertexBuffer();
    void UnbindIndexBuffer();

    const std::vector<float>& RawVertices() const;
    const std::vector<uint32_t>& RawIndices() const;
    const std::vector<Vertex>& Vertices() const;
    const std::vector<Index>& Indices() const;
    const std::vector<uint32_t>& RawLayout() const;
    const Layout& GetLayout() const;

   private:
    friend class Model;
    friend class StaticModel;
    friend class ModelFactory;

    size_t models_produced = 0;

    std::vector<SubMesh> submeshes;

    Ref<VertexBuffer> vertex_buffer;
    Ref<VertexBuffer> index_buffer;
    Ref<VertexBuffer> bone_infl_buffer;

    std::vector<float> fvertices;
    std::vector<uint32_t> raw_indices;
    std::vector<Vertex> vertices;
    std::vector<Index> indices;
    std::vector<uint32_t> raw_layout;
    Layout layout;

    std::vector<Bone> bones;
    std::vector<BoneInfl> bone_influences;
    // std::map<uint32_t , std::vector<Triangle>> triangles;
    // mutable Scope<Skeleton> skeleton = nullptr;

    BBox bounding_box;
    std::string file_path;

    std::vector<MeshNode> nodes;

    void BuildVertexBuffer(const std::vector<Vertex>& vertices);
    void SetLayout();
  };

  class Model : public Asset {
   public:
    OE_ASSET(MODEL);

    explicit Model(Ref<ModelSource>& mesh_source);
    Model(Ref<ModelSource>& mesh_src, const std::vector<uint32_t>& sub_meshes);
    Model(const Ref<Model>& other);
    virtual ~Model() {}

    const std::vector<uint32_t>& SubMeshes() const;
    void SetSubMeshes(const std::vector<uint32_t>& sub_meshes);
    Ref<ModelSource> GetModelSource() const;

    void RebuildMesh();

    /// 1 vertex array from each submesh
    std::vector<Ref<VertexArray>> model_vaos = {};

   private:
    Ref<ModelSource> model_source;
    std::vector<uint32_t> sub_meshes;
  };

  class StaticModel : public Asset {
   public:
    OE_ASSET(MODEL);

    explicit StaticModel(Ref<ModelSource>& mesh_source);
    StaticModel(const Ref<StaticModel>& other);
    virtual ~StaticModel() {}

    Ref<VertexArray> GetMesh() const;
    Ref<ModelSource> GetModelSource() const;

    void RebuildMesh();

    Ref<VertexArray> model_vao = nullptr;

   private:
    Ref<ModelSource> model_source;
  };

}  // namespace other

#endif  // !OTHER_ENGINE_MODEL_HPP
