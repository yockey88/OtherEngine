#ifndef OTHER_ENGINE_VERTEX_HPP
#define OTHER_ENGINE_VERTEX_HPP

#include <cstdint>
#include <vector>

#include "core/defines.hpp"
#include "core/ref.hpp"
#include "core/ref_counted.hpp"
#include "core/uuid.hpp"
#include "math/bounding_box.hpp"
#include "math/vecmath.hpp"

#include "rendering/layout.hpp"
#include "rendering/material.hpp"
#include "rendering/rendering_defines.hpp"

namespace other {

  struct Vertex {
    Point position{ 0.f, 0.f, 0.f };
    glm::vec3 normal{ 0.f, 0.f, 0.f };
    glm::vec3 tangent{ 0.f, 0.f, 0.f };
    glm::vec3 bitangent{ 0.f, 0.f, 0.f };
    glm::vec2 uv_coord{ 0.f, 0.f };

    /// DO NOT FORGET TO CHANGE BOTH OF THESE FUNCTIONS IF YOU CHANGE ONE
    constexpr static std::vector<uint32_t> RawLayout() {
      return { 3, 3, 3, 3, 2 };
    }

    static Layout Layout() {
      return {
        { other::ValueType::VEC3, "position" },
        { other::ValueType::VEC3, "normal" },
        { other::ValueType::VEC3, "tangent" },
        { other::ValueType::VEC3, "bitangent" },
        { other::ValueType::VEC2, "uvs" },
      };
    };

    constexpr static uint32_t Stride() {
      return 14;
    }
  };

  struct Bone {
    glm::mat4 sub_mesh_inverse = glm::mat4(1.f);
    glm::mat4 inverse_bind_pose = glm::mat4(1.f);
    uint32_t sub_mesh_idx = 0;
    uint32_t bone_idx = 0;
  };

  struct BoneInfl {
    uint32_t bone_info_indices[4] = { 0, 0, 0, 0 };
    float weights[4] = { 0.f, 0.f, 0.f, 0.f };

    /// void AddBoneData(uint32_t idx , float weight) {}
    /// void NormalizeWeights() {}
  };

  static constexpr int32_t num_attributes = 5;

  struct Index {
    uint32_t v1, v2, v3;
  };

  struct SubMesh {
    uint32_t base_vertex = 0;
    uint32_t base_idx = 0;
    uint32_t mat_idx = 0;
    uint32_t idx_cnt = 0;
    uint32_t vert_cnt = 0;

    DrawMode topology = DrawMode::TRIANGLES;

    glm::mat4 transform{ 0.f };
    glm::mat4 local_transform{ 0.f };
    BBox bounds{};
    Material material{};

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

  class VertexBuffer : public RefCounted {
   public:
    VertexBuffer(BufferType type, size_t capacity);
    VertexBuffer(const void* data, uint32_t size, BufferUsage usage = STATIC_DRAW, BufferType type = ARRAY_BUFFER);
    // VertexBuffer(const Ref<VertexBuffer>& other);
    ~VertexBuffer();

    size_t Size() const;

    void Bind() const;
    void Unbind() const;

    void BufferData(const void* data, uint32_t size, uint32_t offset = 0);
    void ClearBuffer();

   private:
    BufferType buffer_type;
    BufferUsage buffer_usage;

    Opt<size_t> dynamic_capacity = std::nullopt;

    uint32_t renderer_id = 0;
    uint32_t buffer_size;
  };

  class VertexArray : public RefCounted {
   public:
    VertexArray();
    VertexArray(const std::vector<float>& vertices, const std::vector<uint32_t>& indices = {}, const std::vector<uint32_t>& layout = {});
    VertexArray(const Ref<VertexArray>& other);

    ~VertexArray();

    void Bind() const;
    void Draw(DrawMode mode) const;
    void Unbind() const;

    uint32_t RendererId() const;

    size_t NumElements() const;

    static DrawMode DrawModeFromFaceIndexCount(uint32_t);

   private:
    uint32_t renderer_id;
    uint32_t vertex_count;

    std::vector<float> vertices;
    std::vector<uint32_t> indices;
    std::vector<uint32_t> layout;

    Ref<VertexBuffer> vertex_buffer = nullptr;
    Ref<VertexBuffer> index_buffer = nullptr;

    void SetLayout();
  };

}  // namespace other

#endif  // !OTHER_ENGINE_VERTEX_HPP
