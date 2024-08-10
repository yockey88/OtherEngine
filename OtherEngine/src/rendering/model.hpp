/**
 * \file rendering/model.hpp
 **/
#ifndef OTHER_ENGINE_MODEL_HPP
#define OTHER_ENGINE_MODEL_HPP

#include <glm/glm.hpp>

#include "core/uuid.hpp"
#include "math/aabb.hpp"
#include "asset/asset.hpp"

#include "rendering/rendering_defines.hpp"
#include "rendering/vertex.hpp"

namespace other {

  struct Bone {
    glm::mat4 sub_mesh_inverse = glm::mat4(1.f);
    glm::mat4 inverse_bind_pose = glm::mat4(1.f);
    uint32_t sub_mesh_idx = 0;
    uint32_t bone_idx = 0;
  };

  /// bone serializer?


  struct BoneInfl {
    uint32_t bone_info_indices[4] = { 0 , 0 , 0 , 0 };
    float weights[4] = { 0.f , 0.f , 0.f , 0.f };

    /// void AddBoneData(uint32_t idx , float weight) {}
    /// void NormalizeWeights() {}
  };

  static constexpr int32_t num_attributes = 5;

  struct Index {
    uint32_t v1 , v2 , v3;
  };

  class SubMesh {
    public:
      uint32_t base_vertex = 0;
      uint32_t base_idx = 0;
      uint32_t mat_idx = 0;
      uint32_t idx_cnt = 0;
      uint32_t vert_cnt = 0;

      glm::mat4 transform{ 0.f };
      glm::mat4 local_transform{ 0.f };
      AABB bounds{};

      UUID sub_mesh_id;
      std::string model_name;

      bool rigged = false;
  };

  struct MeshNode {
    uint32_t parent = 0xFFFFFFFF;
    std::vector<uint32_t> children;
    std::vector<uint32_t> sub_meshes;

    std::string name;
    glm::mat4 local_transform;

    inline bool Root() const {
      return parent == 0xFFFFFFFF;
    }
  };

  class ModelSource : public Asset {
    public:
      ModelSource() {}
      ModelSource(std::vector<Vertex>& vertices, std::vector<Index>& indices, const glm::mat4& transform);
      ModelSource(std::vector<Vertex>& vertices, std::vector<Index>& indices, std::vector<SubMesh>& submeshes);

      virtual ~ModelSource() {}

      std::vector<SubMesh>& SubMeshes();
      const std::vector<SubMesh>& SubMeshes() const;

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
      // mutable Scope<Skeleton> skeleton = nullptr;

      // std::vector<Ref<Material>> materials;
      // std::map<uint32_t , std::vector<Triangle>> triangles;
      
      AABB bounding_box;
      std::string file_path;

      std::vector<MeshNode> nodes;

      void BuildVertexBuffer(const std::vector<Vertex>& vertices);
      void SetLayout();
  };

  class Model : public Asset {
    public:
      explicit Model(Ref<ModelSource>& mesh_source);
      Model(Ref<ModelSource>& mesh_src , const std::vector<uint32_t>& sub_meshes);
      Model(const Ref<Model>& other);
      virtual ~Model() {}
      
      const Ref<VertexArray>& GetMesh() const;

      const std::vector<uint32_t>& SubMeshes() const;

      void SetSubMeshes(const std::vector<uint32_t>& sub_meshes);

      void RebuildMesh();

      Ref<ModelSource> GetModelSource();
      Ref<ModelSource> GetModelSource() const;

      void SetModelAsset(Ref<ModelSource> model_src);

    private:
      Ref<ModelSource> model_source;
      Ref<VertexArray> model_vao = nullptr;
      std::vector<uint32_t> sub_meshes;

      // Ref<MaterialTable> material_table;
  };

  class StaticModel : public Asset {
    public:
      explicit StaticModel(Ref<ModelSource>& mesh_source);
      StaticModel(Ref<ModelSource>& mesh_src , const std::vector<uint32_t>& sub_meshes);
      StaticModel(const Ref<StaticModel>& other);
      virtual ~StaticModel() {}

      const Ref<VertexArray>& GetMesh() const;

      const std::vector<uint32_t>& SubMeshes() const;

      void SetSubMeshes(const std::vector<uint32_t>& sub_meshes);
      
      void RebuildMesh();

      Ref<ModelSource> GetModelSource();
      Ref<ModelSource> GetModelSource() const;

      void SetModelAsset(Ref<ModelSource>& mesh_src);

    private:
      Ref<ModelSource> model_source;
      Ref<VertexArray> model_vao = nullptr;
      std::vector<uint32_t> sub_meshes;

      /// Ref<MaterialTable> material_table;
  };

} // namespace other

#endif // !OTHER_ENGINE_MODEL_HPP
